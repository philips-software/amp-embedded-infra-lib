#include "services/network_instantiations/DatagramBsd.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "services/network_instantiations/EventDispatcherWithNetworkBsd.hpp"
#include "services/network_instantiations/SocketAddress.hpp"
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace services
{
    using detail::AddressFamily;
    using detail::AnyAddressSocket;
    using detail::FillSocketAddress;

    DatagramBsd::DatagramBsd(uint16_t port, DatagramExchangeObserver& observer, IPVersions versions)
    {
        observer.Attach(*this);
        family = AddressFamily(versions);
        dualStack = versions == IPVersions::both;
        InitSocket();
        BindLocal(AnyAddressSocket(family, port));
    }

    DatagramBsd::DatagramBsd(DatagramExchangeObserver& observer, IPVersions versions)
    {
        observer.Attach(*this);
        family = AddressFamily(versions);
        dualStack = versions == IPVersions::both;
        InitSocket();
        BindLocal(AnyAddressSocket(family, 0));
    }

    DatagramBsd::DatagramBsd(const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(remote);
        InitSocket();
        BindLocal(AnyAddressSocket(family, 0));
        BindRemote(remote);
    }

    DatagramBsd::DatagramBsd(uint16_t localPort, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(remote);
        InitSocket();
        BindLocal(AnyAddressSocket(family, localPort));
        BindRemote(remote);
    }

    DatagramBsd::DatagramBsd(IPAddress localAddress, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(localAddress);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
    }

    DatagramBsd::DatagramBsd(IPAddress localAddress, uint16_t localPort, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(localAddress);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, localPort));
    }

    DatagramBsd::DatagramBsd(IPAddress localAddress, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(localAddress);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
        BindRemote(remote);
    }

    DatagramBsd::DatagramBsd(const UdpSocket& local, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(local);
        InitSocket();
        BindLocal(local);
        BindRemote(remote);
    }

    DatagramBsd::~DatagramBsd()
    {
        if (HasObserver())
            GetObserver().Detach();

        int result = close(socket);
        if (result == -1)
            std::abort();

        if (socketAdditional != -1 && close(socketAdditional) == -1)
            std::abort();
    }

    bool DatagramBsd::SendBufferEmpty() const
    {
        return sendBuffer == std::nullopt || sendBuffer->empty();
    }

    void DatagramBsd::RequestSendStream(std::size_t sendSize)
    {
        RequestSendStream(sendSize, *connectedTo);
    }

    void DatagramBsd::RequestSendStream(std::size_t sendSize, UdpSocket to)
    {
        assert(streamWriter.Allocatable());
        requestedSendSize = sendSize;
        requestedTo = to;
        TryAllocateSendStream();
    }

    void DatagramBsd::Receive(int socketToReceive)
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<508> receiveBuffer;
        receiveBuffer.resize(receiveBuffer.max_size());

        sockaddr_storage fromAddress{};
        socklen_t fromAddressSize = sizeof(fromAddress);

        auto received = recvfrom(socketToReceive, reinterpret_cast<char*>(receiveBuffer.data()), receiveBuffer.size(), 0, reinterpret_cast<sockaddr*>(&fromAddress), &fromAddressSize);
        if (received == -1)
        {
            if (errno != EWOULDBLOCK && errno != EMSGSIZE)
                std::abort();
            return;
        }

        receiveBuffer.resize(received);
        auto reader = infra::MakeSharedOnHeap<infra::StdVectorInputStreamReader::WithStorage>();
        reader->Storage() = std::vector<uint8_t>(receiveBuffer.begin(), receiveBuffer.end());

        if (fromAddress.ss_family == AF_INET)
        {
            auto& from4 = reinterpret_cast<sockaddr_in&>(fromAddress);
            auto from = Udpv4Socket{ services::ConvertFromUint32(htonl(from4.sin_addr.s_addr)), htons(from4.sin_port) };
            GetObserver().DataReceived(std::move(reader), from);
        }
        else if (fromAddress.ss_family == AF_INET6)
        {
            auto& from6 = reinterpret_cast<sockaddr_in6&>(fromAddress);
            IPv6AddressNetworkOrder networkOrder;
            std::memcpy(networkOrder.data(), &from6.sin6_addr, sizeof(from6.sin6_addr));
            auto from = Udpv6Socket{ services::FromNetworkOrder(networkOrder), htons(from6.sin6_port) };
            GetObserver().DataReceived(std::move(reader), from);
        }
    }

    void DatagramBsd::Send()
    {
        sockaddr_storage address{};
        auto addressSize = FillSocketAddress(requestedTo, address);

        auto sent = sendto(SocketFor(requestedTo), reinterpret_cast<char*>(sendBuffer->data()), sendBuffer->size(), 0, reinterpret_cast<sockaddr*>(&address), addressSize);

        if (sent == -1)
        {
            if (errno != EWOULDBLOCK)
                std::abort();
            return;
        }

        sendBuffer.reset();
        TryAllocateSendStream();
    }

    void DatagramBsd::TrySend()
    {
        if (trySend)
        {
            Send();
            trySend = false;
            self = nullptr;
        }
    }

    void DatagramBsd::JoinMulticastGroup(IPv4Address multicastAddress)
    {
        struct ip_mreq multicastRequest;
        if (localAddress == IPv4Address())
            multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
        else
            multicastRequest.imr_interface.s_addr = htonl(services::ConvertToUint32(localAddress));
        multicastRequest.imr_multiaddr.s_addr = htonl(services::ConvertToUint32(multicastAddress));

        auto result = setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
        assert(result == 0);
    }

    void DatagramBsd::LeaveMulticastGroup(IPv4Address multicastAddress)
    {
        struct ip_mreq multicastRequest;
        if (localAddress == IPv4Address())
            multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
        else
            multicastRequest.imr_interface.s_addr = htonl(services::ConvertToUint32(localAddress));
        multicastRequest.imr_multiaddr.s_addr = htonl(services::ConvertToUint32(multicastAddress));

        auto result = setsockopt(socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
        assert(result == 0);
    }

    void DatagramBsd::JoinMulticastGroup(IPv6Address multicastAddress)
    {
        ipv6_mreq multicastRequest{};
        auto networkOrder = services::ToNetworkOrder(multicastAddress);
        std::memcpy(&multicastRequest.ipv6mr_multiaddr, networkOrder.data(), sizeof(multicastRequest.ipv6mr_multiaddr));
        multicastRequest.ipv6mr_interface = 0;

        auto result = setsockopt(Ipv6Socket(), IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
        assert(result == 0);
    }

    void DatagramBsd::LeaveMulticastGroup(IPv6Address multicastAddress)
    {
        ipv6_mreq multicastRequest{};
        auto networkOrder = services::ToNetworkOrder(multicastAddress);
        std::memcpy(&multicastRequest.ipv6mr_multiaddr, networkOrder.data(), sizeof(multicastRequest.ipv6mr_multiaddr));
        multicastRequest.ipv6mr_interface = 0;

        auto result = setsockopt(Ipv6Socket(), IPPROTO_IPV6, IPV6_LEAVE_GROUP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
        assert(result == 0);
    }

    void DatagramBsd::InitSocket()
    {
        socket = ::socket(family, SOCK_DGRAM, IPPROTO_UDP);
        assert(socket != -1);

        int flag = 1;
        if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
            std::abort();

        if (family == AF_INET6)
        {
            int v6Only = 1;
            if (setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, &v6Only, sizeof(v6Only)) == -1)
                std::abort();
        }

        if (fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK) == -1)
            std::abort();

        if (dualStack)
        {
            socketAdditional = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
            assert(socketAdditional != -1);

            if (setsockopt(socketAdditional, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
                std::abort();

            int v6Only = 1;
            if (setsockopt(socketAdditional, IPPROTO_IPV6, IPV6_V6ONLY, &v6Only, sizeof(v6Only)) == -1)
                std::abort();

            if (fcntl(socketAdditional, F_SETFL, fcntl(socketAdditional, F_GETFL, 0) | O_NONBLOCK) == -1)
                std::abort();
        }
    }

    int DatagramBsd::SocketFor(const UdpSocket& to) const
    {
        if (std::holds_alternative<Udpv6Socket>(to))
            return Ipv6Socket();
        return socket;
    }

    int DatagramBsd::Ipv6Socket() const
    {
        return family == AF_INET6 ? socket : socketAdditional;
    }

    void DatagramBsd::BindLocal(const UdpSocket& local)
    {
        if (std::holds_alternative<Udpv4Socket>(local))
            localAddress = std::get<Udpv4Socket>(local).first;
        else
            localAddress = IPv4Address{};

        sockaddr_storage address{};
        auto addressSize = FillSocketAddress(local, address);
        auto result = bind(socket, reinterpret_cast<sockaddr*>(&address), addressSize);
        assert(result == 0);

        if (socketAdditional != -1)
        {
            uint16_t port = std::holds_alternative<Udpv4Socket>(local) ? std::get<Udpv4Socket>(local).second : std::get<Udpv6Socket>(local).second;
            sockaddr_storage address6{};
            auto address6Size = FillSocketAddress(AnyAddressSocket(AF_INET6, port), address6);
            auto result6 = bind(socketAdditional, reinterpret_cast<sockaddr*>(&address6), address6Size);
            assert(result6 == 0);
        }
    }

    void DatagramBsd::BindRemote(const UdpSocket& remote)
    {
        sockaddr_storage address{};
        auto addressSize = FillSocketAddress(remote, address);
        auto result = connect(socket, reinterpret_cast<sockaddr*>(&address), addressSize);
        assert(result == 0);

        connectedTo = remote;
    }

    void DatagramBsd::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (!sendBuffer && requestedSendSize != 0)
        {
            sendBuffer.emplace(requestedSendSize, 0);
            requestedSendSize = 0;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<DatagramBsd>& object)
                {
                    infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object);
                    object->GetObserver().SendStreamAvailable(std::move(writer));
                },
                SharedFromThis());
        }
    }

    DatagramBsd::StreamWriterBsd::StreamWriterBsd(DatagramBsd& connection)
        : infra::ByteOutputStreamWriter(infra::MakeRange(*connection.sendBuffer))
        , connection(connection)
    {}

    DatagramBsd::StreamWriterBsd::~StreamWriterBsd()
    {
        connection.sendBuffer->resize(Processed().size());
        connection.trySend = true;
        connection.self = connection.SharedFromThis();
    }
}
