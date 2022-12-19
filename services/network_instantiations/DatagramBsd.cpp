#include "services/network_instantiations/DatagramBsd.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "services/network_instantiations/EventDispatcherWithNetworkBsd.hpp"
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace services
{
    DatagramBsd::DatagramBsd(uint16_t port, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, port });
    }

    DatagramBsd::DatagramBsd(DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, 0 });
    }

    DatagramBsd::DatagramBsd(const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, 0 });
        BindRemote(remote);
    }

    DatagramBsd::DatagramBsd(uint16_t localPort, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, localPort });
        BindRemote(remote);
    }

    DatagramBsd::DatagramBsd(IPAddress localAddress, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
    }

    DatagramBsd::DatagramBsd(IPAddress localAddress, uint16_t localPort, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, localPort));
    }

    DatagramBsd::DatagramBsd(IPAddress localAddress, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
        BindRemote(remote);
    }

    DatagramBsd::DatagramBsd(const UdpSocket& local, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
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

    void DatagramBsd::Receive()
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<508> receiveBuffer;
        receiveBuffer.resize(receiveBuffer.max_size());

        sockaddr_in fromAddress{};
        socklen_t fromAddressSize = sizeof(fromAddress);

        auto received = recvfrom(socket, reinterpret_cast<char*>(receiveBuffer.data()), receiveBuffer.size(), 0, reinterpret_cast<sockaddr*>(&fromAddress), &fromAddressSize);
        if (received == -1)
        {
            if (errno != EWOULDBLOCK && errno != EMSGSIZE)
                std::abort();
            return;
        }
        else if (fromAddressSize == sizeof(fromAddress) && fromAddress.sin_family == AF_INET)
        {
            receiveBuffer.resize(received);
            auto reader = infra::MakeSharedOnHeap<infra::StdVectorInputStreamReader::WithStorage>();
            reader->Storage() = std::vector<uint8_t>(receiveBuffer.begin(), receiveBuffer.end());

            auto from = Udpv4Socket{ services::ConvertFromUint32(htonl(fromAddress.sin_addr.s_addr)), htons(fromAddress.sin_port) };

            GetObserver().DataReceived(std::move(reader), from);
        }
    }

    void DatagramBsd::Send()
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(requestedTo.Get<Udpv4Socket>().first));
        address.sin_port = htons(requestedTo.Get<Udpv4Socket>().second);
        int sent = sendto(socket, reinterpret_cast<char*>(sendBuffer->data()), sendBuffer->size(), 0, reinterpret_cast<sockaddr*>(&address), sizeof(address));

        if (sent == -1)
        {
            if (errno != EWOULDBLOCK)
                std::abort();
            return;
        }

        sendBuffer = infra::none;
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

        setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
    }

    void DatagramBsd::LeaveMulticastGroup(IPv4Address multicastAddress)
    {
        struct ip_mreq multicastRequest;
        if (localAddress == IPv4Address())
            multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
        else
            multicastRequest.imr_interface.s_addr = htonl(services::ConvertToUint32(localAddress));
        multicastRequest.imr_multiaddr.s_addr = htonl(services::ConvertToUint32(multicastAddress));

        setsockopt(socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
    }

    void DatagramBsd::InitSocket()
    {
        assert(socket != -1);

        int flag = 1;
        if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
            std::abort();

        if (fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK) == -1)
            std::abort();
    }

    void DatagramBsd::BindLocal(const UdpSocket& local)
    {
        localAddress = local.Get<Udpv4Socket>().first;
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(localAddress));
        address.sin_port = htons(local.Get<Udpv4Socket>().second);
        auto result = bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        assert(result == 0);
    }

    void DatagramBsd::BindRemote(const UdpSocket& remote)
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(remote.Get<Udpv4Socket>().first));
        address.sin_port = htons(remote.Get<Udpv4Socket>().second);
        auto result = connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        assert(result == 0);

        connectedTo = remote;
    }

    void DatagramBsd::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (!sendBuffer && requestedSendSize != 0)
        {
            sendBuffer.Emplace(requestedSendSize, 0);
            requestedSendSize = 0;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<DatagramBsd>& object)
                {
                infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object);
                object->GetObserver().SendStreamAvailable(std::move(writer)); },
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
