#include "services/network_instantiations/DatagramWin.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "services/network_instantiations/EventDispatcherWithNetworkWin.hpp"
#include "services/network_instantiations/SocketAddress.hpp"
#include <cstring>
#include <ws2tcpip.h>

namespace services
{
    using detail::AddressFamily;
    using detail::AnyAddressSocket;
    using detail::FillSocketAddress;

    DatagramWin::DatagramWin(uint16_t port, DatagramExchangeObserver& observer, IPVersions versions)
    {
        observer.Attach(*this);
        family = AddressFamily(versions);
        dualStack = versions == IPVersions::both;
        InitSocket();
        BindLocal(AnyAddressSocket(family, port));
    }

    DatagramWin::DatagramWin(DatagramExchangeObserver& observer, IPVersions versions)
    {
        observer.Attach(*this);
        family = AddressFamily(versions);
        dualStack = versions == IPVersions::both;
        InitSocket();
        BindLocal(AnyAddressSocket(family, 0));
    }

    DatagramWin::DatagramWin(const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(remote);
        InitSocket();
        BindLocal(AnyAddressSocket(family, 0));
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(uint16_t localPort, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(remote);
        InitSocket();
        BindLocal(AnyAddressSocket(family, localPort));
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(IPAddress localAddress, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(localAddress);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
    }

    DatagramWin::DatagramWin(IPAddress localAddress, uint16_t localPort, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(localAddress);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, localPort));
    }

    DatagramWin::DatagramWin(IPv6Address localAddress, uint32_t interfaceIndex, uint16_t localPort, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AF_INET6;
        ipv6InterfaceIndex = interfaceIndex;
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, localPort));
    }

    DatagramWin::DatagramWin(IPAddress localAddress, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(localAddress);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(const UdpSocket& local, const UdpSocket& remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        family = AddressFamily(local);
        InitSocket();
        BindLocal(local);
        BindRemote(remote);
    }

    DatagramWin::~DatagramWin()
    {
        if (HasObserver())
            GetObserver().Detach();

        BOOL result = WSACloseEvent(event);
        assert(result == TRUE);
        result = closesocket(socket);
        if (result == SOCKET_ERROR)
        {
            DWORD error = GetLastError();
            std::abort();
        }

        if (socketAdditional != INVALID_SOCKET)
        {
            WSACloseEvent(eventAdditional);
            if (closesocket(socketAdditional) == SOCKET_ERROR)
                std::abort();
        }
    }

    void DatagramWin::RequestSendStream(std::size_t sendSize)
    {
        RequestSendStream(sendSize, *connectedTo);
    }

    void DatagramWin::RequestSendStream(std::size_t sendSize, UdpSocket to)
    {
        assert(streamWriter.Allocatable());
        requestedSendSize = sendSize;
        requestedTo = to;
        TryAllocateSendStream();
    }

    void DatagramWin::Receive(SOCKET socketToReceive)
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<508> receiveBuffer;
        receiveBuffer.resize(receiveBuffer.max_size());

        sockaddr_storage fromAddress{};
        int fromAddressSize = sizeof(fromAddress);

        auto received = recvfrom(socketToReceive, reinterpret_cast<char*>(receiveBuffer.data()), receiveBuffer.size(), 0, reinterpret_cast<sockaddr*>(&fromAddress), &fromAddressSize);
        if (received == SOCKET_ERROR)
        {
            auto error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAEMSGSIZE)
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

    void DatagramWin::Send()
    {
        UpdateEventFlags(); // If there is something to send, update the flags before calling send, because FD_SEND is an edge-triggered event.
        sockaddr_storage address{};
        auto addressSize = FillSocketAddress(requestedTo, address);

        auto sent = sendto(SocketFor(requestedTo), reinterpret_cast<char*>(sendBuffer->data()), sendBuffer->size(), 0, reinterpret_cast<sockaddr*>(&address), addressSize);

        if (sent == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                std::abort();
            return;
        }

        sendBuffer.reset();
        TryAllocateSendStream();
        UpdateEventFlags();
    }

    void DatagramWin::TrySend()
    {
        if (trySend)
        {
            Send();
            trySend = false;
            self = nullptr;
        }
    }

    void DatagramWin::UpdateEventFlags()
    {
        int result = WSAEventSelect(socket, event, FD_READ | (sendBuffer ? FD_WRITE : 0));
        assert(result == 0);

        if (socketAdditional != INVALID_SOCKET)
        {
            result = WSAEventSelect(socketAdditional, eventAdditional, FD_READ | (sendBuffer ? FD_WRITE : 0));
            assert(result == 0);
        }
    }

    void DatagramWin::JoinMulticastGroup(IPv4Address multicastAddress)
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

    void DatagramWin::LeaveMulticastGroup(IPv4Address multicastAddress)
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

    void DatagramWin::JoinMulticastGroup(IPv6Address multicastAddress)
    {
        ipv6_mreq multicastRequest{};
        auto networkOrder = services::ToNetworkOrder(multicastAddress);
        std::memcpy(&multicastRequest.ipv6mr_multiaddr, networkOrder.data(), sizeof(multicastRequest.ipv6mr_multiaddr));
        multicastRequest.ipv6mr_interface = ipv6InterfaceIndex;

        auto result = setsockopt(Ipv6Socket(), IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
        assert(result == 0);
    }

    void DatagramWin::LeaveMulticastGroup(IPv6Address multicastAddress)
    {
        ipv6_mreq multicastRequest{};
        auto networkOrder = services::ToNetworkOrder(multicastAddress);
        std::memcpy(&multicastRequest.ipv6mr_multiaddr, networkOrder.data(), sizeof(multicastRequest.ipv6mr_multiaddr));
        multicastRequest.ipv6mr_interface = ipv6InterfaceIndex;

        auto result = setsockopt(Ipv6Socket(), IPPROTO_IPV6, IPV6_LEAVE_GROUP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
        assert(result == 0);
    }

    void DatagramWin::InitSocket()
    {
        socket = ::socket(family, SOCK_DGRAM, IPPROTO_UDP);
        assert(socket != INVALID_SOCKET);

        std::array<char, 1> option{ true };
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, option.data(), option.size());

        if (family == AF_INET6)
        {
            DWORD v6Only = 1;
            if (setsockopt(socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&v6Only), sizeof(v6Only)) == SOCKET_ERROR)
                std::abort();
        }

        ULONG nonBlock = 1;
        if (ioctlsocket(socket, FIONBIO, &nonBlock) == SOCKET_ERROR)
            std::abort();

        if (dualStack)
        {
            socketAdditional = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
            assert(socketAdditional != INVALID_SOCKET);
            eventAdditional = WSACreateEvent();

            setsockopt(socketAdditional, SOL_SOCKET, SO_REUSEADDR, option.data(), option.size());

            DWORD v6Only = 1;
            if (setsockopt(socketAdditional, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&v6Only), sizeof(v6Only)) == SOCKET_ERROR)
                std::abort();

            if (ioctlsocket(socketAdditional, FIONBIO, &nonBlock) == SOCKET_ERROR)
                std::abort();
        }

        UpdateEventFlags();
    }

    SOCKET DatagramWin::SocketFor(const UdpSocket& to) const
    {
        if (std::holds_alternative<Udpv6Socket>(to))
            return Ipv6Socket();
        return socket;
    }

    SOCKET DatagramWin::Ipv6Socket() const
    {
        return family == AF_INET6 ? socket : socketAdditional;
    }

    void DatagramWin::BindLocal(const UdpSocket& local)
    {
        if (std::holds_alternative<Udpv4Socket>(local))
            localAddress = std::get<Udpv4Socket>(local).first;
        else
            localAddress = IPv4Address{};

        sockaddr_storage address{};
        auto addressSize = FillSocketAddress(local, address);
        auto result = bind(socket, reinterpret_cast<sockaddr*>(&address), addressSize);
        assert(result == 0);

        if (socketAdditional != INVALID_SOCKET)
        {
            uint16_t port = std::holds_alternative<Udpv4Socket>(local) ? std::get<Udpv4Socket>(local).second : std::get<Udpv6Socket>(local).second;
            sockaddr_storage address6{};
            auto address6Size = FillSocketAddress(AnyAddressSocket(AF_INET6, port), address6);
            auto result6 = bind(socketAdditional, reinterpret_cast<sockaddr*>(&address6), address6Size);
            assert(result6 == 0);
        }
    }

    void DatagramWin::BindRemote(const UdpSocket& remote)
    {
        sockaddr_storage address{};
        auto addressSize = FillSocketAddress(remote, address);
        auto result = connect(socket, reinterpret_cast<sockaddr*>(&address), addressSize);
        assert(result == 0);

        connectedTo = remote;
    }

    void DatagramWin::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (!sendBuffer && requestedSendSize != 0)
        {
            sendBuffer.emplace(requestedSendSize, 0);
            requestedSendSize = 0;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<DatagramWin>& object)
                {
                    infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object);
                    object->GetObserver().SendStreamAvailable(std::move(writer));
                },
                SharedFromThis());
        }
    }

    DatagramWin::StreamWriterWin::StreamWriterWin(DatagramWin& connection)
        : infra::ByteOutputStreamWriter(infra::MakeRange(*connection.sendBuffer))
        , connection(connection)
    {}

    DatagramWin::StreamWriterWin::~StreamWriterWin()
    {
        connection.sendBuffer->resize(Processed().size());
        connection.trySend = true;
        connection.self = connection.SharedFromThis();
    }

    DatagramExchangeMultiple::DatagramExchangeMultiple(DatagramExchangeObserver& observer, EventDispatcherWithNetwork& eventDispatcher)
        : eventDispatcher(eventDispatcher)
    {
        observer.Attach(*this);
    }

    DatagramExchangeMultiple::~DatagramExchangeMultiple()
    {
        GetObserver().Detach();
    }

    void DatagramExchangeMultiple::Add(DatagramFactoryWithLocalIpBinding& factory, IPAddress local, uint16_t port, IPVersions versions)
    {
        observers.push_back(infra::MakeSharedOnHeap<Observer>(*this, factory, local, port, versions));
    }

    void DatagramExchangeMultiple::Add(DatagramFactoryWithLocalIpBinding& factory, IPAddress local, IPVersions versions)
    {
        observers.push_back(infra::MakeSharedOnHeap<Observer>(*this, factory, local, versions));
    }

    void DatagramExchangeMultiple::Add(DatagramFactoryWithLocalIpBinding& factory, IPv6Address local, uint32_t interfaceIndex, uint16_t port)
    {
        observers.push_back(infra::MakeSharedOnHeap<Observer>(*this, factory, local, interfaceIndex, port));
    }

    void DatagramExchangeMultiple::Add(DatagramFactoryWithLocalIpBinding& factory, IPAddress local, UdpSocket remote)
    {
        observers.push_back(infra::MakeSharedOnHeap<Observer>(*this, factory, local, remote));
    }

    void DatagramExchangeMultiple::Add(DatagramFactoryWithLocalIpBinding& factory, UdpSocket local, UdpSocket remote)
    {
        observers.push_back(infra::MakeSharedOnHeap<Observer>(*this, factory, local, remote));
    }

    void DatagramExchangeMultiple::JoinMulticastGroup(IPv4Address multicastAddress)
    {
        for (const auto& observer : observers)
            if (observer->version == IPVersions::ipv4)
                eventDispatcher.JoinMulticastGroup(observer->exchange, multicastAddress);
    }

    void DatagramExchangeMultiple::LeaveMulticastGroup(IPv4Address multicastAddress)
    {
        for (const auto& observer : observers)
            if (observer->version == IPVersions::ipv4)
                eventDispatcher.LeaveMulticastGroup(observer->exchange, multicastAddress);
    }

    void DatagramExchangeMultiple::JoinMulticastGroup(IPv6Address multicastAddress)
    {
        for (const auto& observer : observers)
            if (observer->version == IPVersions::ipv6)
                eventDispatcher.JoinMulticastGroup(observer->exchange, multicastAddress);
    }

    void DatagramExchangeMultiple::LeaveMulticastGroup(IPv6Address multicastAddress)
    {
        for (const auto& observer : observers)
            if (observer->version == IPVersions::ipv6)
                eventDispatcher.LeaveMulticastGroup(observer->exchange, multicastAddress);
    }

    void DatagramExchangeMultiple::RequestSendStream(std::size_t sendSize)
    {
        assert(writers.empty());

        expectedWriters = observers.size();
        for (auto& observer : observers)
            observer->Subject().RequestSendStream(sendSize);
    }

    void DatagramExchangeMultiple::RequestSendStream(std::size_t sendSize, UdpSocket to)
    {
        assert(writers.empty());

        auto toVersion = std::holds_alternative<Udpv6Socket>(to) ? IPVersions::ipv6 : IPVersions::ipv4;

        expectedWriters = 0;
        for (auto& observer : observers)
            if (observer->version == toVersion)
                ++expectedWriters;

        assert(expectedWriters != 0);

        for (auto& observer : observers)
            if (observer->version == toVersion)
                observer->Subject().RequestSendStream(sendSize, to);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, uint16_t port, IPVersions versions)
        : parent(parent)
        , version(std::holds_alternative<IPv6Address>(local) ? IPVersions::ipv6 : IPVersions::ipv4)
    {
        exchange = factory.Listen(*this, local, port, versions);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, IPVersions versions)
        : parent(parent)
        , version(std::holds_alternative<IPv6Address>(local) ? IPVersions::ipv6 : IPVersions::ipv4)
    {
        exchange = factory.Listen(*this, local, versions);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPv6Address local, uint32_t interfaceIndex, uint16_t port)
        : parent(parent)
        , version(IPVersions::ipv6)
    {
        exchange = factory.Listen(*this, local, interfaceIndex, port);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, UdpSocket remote)
        : parent(parent)
        , version(std::holds_alternative<Udpv6Socket>(remote) ? IPVersions::ipv6 : IPVersions::ipv4)
    {
        exchange = factory.Connect(*this, local, remote);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, UdpSocket local, UdpSocket remote)
        : parent(parent)
        , version(std::holds_alternative<Udpv6Socket>(remote) ? IPVersions::ipv6 : IPVersions::ipv4)
    {
        exchange = factory.Connect(*this, local, remote);
    }

    void DatagramExchangeMultiple::Observer::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from)
    {
        parent.GetObserver().DataReceived(std::move(reader), from);
    }

    void DatagramExchangeMultiple::Observer::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        parent.writers.emplace_back(std::move(writer));

        if (parent.writers.size() == parent.expectedWriters)
        {
            parent.GetObserver().SendStreamAvailable(parent.multipleWriter.Emplace(parent.writers));
        }
    }

    DatagramExchangeMultiple::MultipleWriter::MultipleWriter(std::vector<infra::SharedPtr<infra::StreamWriter>>& writers)
        : writers(writers)
    {
        writers.clear();
    }

    void DatagramExchangeMultiple::MultipleWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        for (auto& writer : writers)
            writer->Insert(range, errorPolicy);
    }

    std::size_t DatagramExchangeMultiple::MultipleWriter::Available() const
    {
        return writers.front()->Available();
    }

    std::size_t DatagramExchangeMultiple::MultipleWriter::ConstructSaveMarker() const
    {
        return writers.front()->ConstructSaveMarker();
    }

    std::size_t DatagramExchangeMultiple::MultipleWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        return writers.front()->GetProcessedBytesSince(marker);
    }

    infra::ByteRange DatagramExchangeMultiple::MultipleWriter::SaveState(std::size_t marker)
    {
        std::abort();
    }

    void DatagramExchangeMultiple::MultipleWriter::RestoreState(infra::ByteRange range)
    {
        std::abort();
    }

    infra::ByteRange DatagramExchangeMultiple::MultipleWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }

    UdpOnAllInterfaces::UdpOnAllInterfaces(EventDispatcherWithNetwork& eventDispatcher)
        : eventDispatcher(eventDispatcher)
    {}

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions)
    {
        auto result = infra::MakeSharedOnHeap<DatagramExchangeMultiple>(observer, eventDispatcher);
        eventDispatcher.RegisterDatagramMultiple(result);

        if (versions != IPVersions::ipv6)
            for (auto address : GetIpAddresses())
                result->Add(eventDispatcher, address, port, versions);

        if (versions != IPVersions::ipv4)
            for (auto [address, interfaceIndex] : GetIpv6Addresses())
                result->Add(eventDispatcher, address, interfaceIndex, port);

        return result;
    }

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Listen(DatagramExchangeObserver& observer, IPVersions versions)
    {
        auto result = infra::MakeSharedOnHeap<DatagramExchangeMultiple>(observer, eventDispatcher);
        eventDispatcher.RegisterDatagramMultiple(result);

        if (versions != IPVersions::ipv6)
            for (auto address : GetIpAddresses())
                result->Add(eventDispatcher, address, versions);

        if (versions != IPVersions::ipv4)
            for (auto [address, interfaceIndex] : GetIpv6Addresses())
                result->Add(eventDispatcher, address, interfaceIndex, 0);

        return result;
    }

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Connect(DatagramExchangeObserver& observer, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramExchangeMultiple>(observer, eventDispatcher);
        eventDispatcher.RegisterDatagramMultiple(result);

        for (auto address : GetIpAddresses())
            result->Add(eventDispatcher, address, remote);

        return result;
    }

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramExchangeMultiple>(observer, eventDispatcher);
        eventDispatcher.RegisterDatagramMultiple(result);

        for (auto address : GetIpAddresses())
            result->Add(eventDispatcher, MakeUdpSocket(address, localPort), remote);

        return result;
    }

    std::vector<IPv4Address> UdpOnAllInterfaces::GetIpAddresses()
    {
        ULONG size(0);
        auto result = GetAdaptersAddresses(AF_INET, 0, nullptr, nullptr, &size);
        assert(result == ERROR_BUFFER_OVERFLOW);
        auto adapterInfo = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(malloc(size));
        auto originalAdapterInfo = adapterInfo;
        result = GetAdaptersAddresses(AF_INET, 0, nullptr, adapterInfo, &size);
        assert(result == NO_ERROR);

        std::vector<IPv4Address> addresses;

        for (; adapterInfo != nullptr; adapterInfo = adapterInfo->Next)
            for (auto ipAddresses = adapterInfo->FirstUnicastAddress; ipAddresses != nullptr; ipAddresses = ipAddresses->Next)
                if (adapterInfo->OperStatus == IfOperStatusUp && ipAddresses->Address.lpSockaddr != nullptr && ipAddresses->Address.lpSockaddr->sa_family == AF_INET)
                {
                    auto address = reinterpret_cast<sockaddr_in&>(*ipAddresses->Address.lpSockaddr);
                    addresses.push_back(IPv4Address{ address.sin_addr.s_net, address.sin_addr.s_host, address.sin_addr.s_lh, address.sin_addr.s_impno });
                }

        free(originalAdapterInfo);

        return addresses;
    }

    std::vector<std::pair<IPv6Address, uint32_t>> UdpOnAllInterfaces::GetIpv6Addresses()
    {
        ULONG size(0);
        auto result = GetAdaptersAddresses(AF_INET6, 0, nullptr, nullptr, &size);
        assert(result == ERROR_BUFFER_OVERFLOW);
        auto adapterInfo = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(malloc(size));
        auto originalAdapterInfo = adapterInfo;
        result = GetAdaptersAddresses(AF_INET6, 0, nullptr, adapterInfo, &size);
        assert(result == NO_ERROR);

        std::vector<std::pair<IPv6Address, uint32_t>> addresses;

        for (; adapterInfo != nullptr; adapterInfo = adapterInfo->Next)
            for (auto ipAddresses = adapterInfo->FirstUnicastAddress; ipAddresses != nullptr; ipAddresses = ipAddresses->Next)
                if (adapterInfo->OperStatus == IfOperStatusUp && ipAddresses->Address.lpSockaddr != nullptr && ipAddresses->Address.lpSockaddr->sa_family == AF_INET6)
                {
                    auto& address = reinterpret_cast<sockaddr_in6&>(*ipAddresses->Address.lpSockaddr);
                    IPv6AddressNetworkOrder networkOrder;
                    std::memcpy(networkOrder.data(), &address.sin6_addr, sizeof(address.sin6_addr));
                    addresses.emplace_back(services::FromNetworkOrder(networkOrder), adapterInfo->Ipv6IfIndex);
                }

        free(originalAdapterInfo);

        return addresses;
    }
}
