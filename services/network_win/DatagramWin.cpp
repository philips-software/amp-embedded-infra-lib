#include "infra/stream/StdVectorInputStream.hpp"
#include "services/network_win/DatagramWin.hpp"
#include "services/network_win/EventDispatcherWithNetwork.hpp"
#include <ws2tcpip.h>

namespace services
{
    DatagramWin::DatagramWin(uint16_t port, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, port });
    }

    DatagramWin::DatagramWin(DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, 0 });
    }

    DatagramWin::DatagramWin(UdpSocket remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, 0 });
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(uint16_t localPort, UdpSocket remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, localPort });
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(IPAddress localAddress, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
    }

    DatagramWin::DatagramWin(IPAddress localAddress, uint16_t localPort, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, localPort));
    }

    DatagramWin::DatagramWin(IPAddress localAddress, UdpSocket remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(UdpSocket local, UdpSocket remote, DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
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

    void DatagramWin::Receive()
    {
        infra::BoundedVector<uint8_t>::WithMaxSize<508> receiveBuffer;
        receiveBuffer.resize(receiveBuffer.max_size());

        sockaddr_in fromAddress{};
        int fromAddressSize = sizeof(fromAddress);

        auto received = recvfrom(socket, reinterpret_cast<char*>(receiveBuffer.data()), receiveBuffer.size(), 0, reinterpret_cast<sockaddr*>(&fromAddress), &fromAddressSize);
        if (received == SOCKET_ERROR)
        {
            auto error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAEMSGSIZE)
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

    void DatagramWin::Send()
    {
        UpdateEventFlags();     // If there is something to send, update the flags before calling send, because FD_SEND is an edge-triggered event.
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(requestedTo.Get<Udpv4Socket>().first));
        address.sin_port = htons(requestedTo.Get<Udpv4Socket>().second);
        int sent = sendto(socket, reinterpret_cast<char*>(sendBuffer->data()), sendBuffer->size(), 0, reinterpret_cast<sockaddr*>(&address), sizeof(address));

        if (sent == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                std::abort();
            return;
        }

        sendBuffer = infra::none;
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
    }

    void DatagramWin::JoinMulticastGroup(IPv4Address multicastAddress)
    {
        struct ip_mreq multicastRequest;
        if (localAddress == IPv4Address())
            multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
        else
            multicastRequest.imr_interface.s_addr = htonl(services::ConvertToUint32(localAddress));
        multicastRequest.imr_multiaddr.s_addr = htonl(services::ConvertToUint32(multicastAddress));

        setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
    }

    void DatagramWin::LeaveMulticastGroup(IPv4Address multicastAddress)
    {
        struct ip_mreq multicastRequest;
        if (localAddress == IPv4Address())
            multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
        else
            multicastRequest.imr_interface.s_addr = htonl(services::ConvertToUint32(localAddress));
        multicastRequest.imr_multiaddr.s_addr = htonl(services::ConvertToUint32(multicastAddress));

        setsockopt(socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<char*>(&multicastRequest), sizeof(multicastRequest));
    }

    void DatagramWin::InitSocket()
    {
        assert(socket != INVALID_SOCKET);

        std::array<char, 1> option{ true };
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, option.data(), option.size());

        ULONG nonBlock = 1;
        if (ioctlsocket(socket, FIONBIO, &nonBlock) == SOCKET_ERROR)
            std::abort();

        UpdateEventFlags();
    }

    void DatagramWin::BindLocal(UdpSocket local)
    {
        localAddress = local.Get<Udpv4Socket>().first;
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(localAddress));
        address.sin_port = htons(local.Get<Udpv4Socket>().second);
        auto result = bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        auto error = WSAGetLastError();
        assert(result == 0);
    }

    void DatagramWin::BindRemote(UdpSocket remote)
    {
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(remote.Get<Udpv4Socket>().first));
        address.sin_port = htons(remote.Get<Udpv4Socket>().second);
        auto result = connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        auto error = WSAGetLastError();
        assert(result == 0);

        connectedTo = remote;
    }

    void DatagramWin::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (!sendBuffer && requestedSendSize != 0)
        {
            sendBuffer.Emplace(requestedSendSize, 0);
            requestedSendSize = 0;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<DatagramWin>& object)
            {
                infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object);
                object->GetObserver().SendStreamAvailable(std::move(writer));
            }, SharedFromThis());
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
            eventDispatcher.JoinMulticastGroup(observer->exchange, multicastAddress);
    }

    void DatagramExchangeMultiple::LeaveMulticastGroup(IPv4Address multicastAddress)
    {
        for (const auto& observer : observers)
            eventDispatcher.LeaveMulticastGroup(observer->exchange, multicastAddress);
    }

    void DatagramExchangeMultiple::RequestSendStream(std::size_t sendSize)
    {
        assert(writers.empty());

        for (auto& observer : observers)
            observer->Subject().RequestSendStream(sendSize);
    }

    void DatagramExchangeMultiple::RequestSendStream(std::size_t sendSize, UdpSocket to)
    {
        assert(writers.empty());

        for (auto& observer : observers)
            observer->Subject().RequestSendStream(sendSize, to);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, uint16_t port, IPVersions versions)
        : parent(parent)
    {
        exchange = factory.Listen(*this, local, port, versions);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, IPVersions versions)
        : parent(parent)
    {
        exchange = factory.Listen(*this, local, versions);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, IPAddress local, UdpSocket remote)
        : parent(parent)
    {
        exchange = factory.Connect(*this, local, remote);
    }

    DatagramExchangeMultiple::Observer::Observer(DatagramExchangeMultiple& parent, DatagramFactoryWithLocalIpBinding& factory, UdpSocket local, UdpSocket remote)
        : parent(parent)
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

        if (parent.writers.size() == parent.observers.size())
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

        for (auto address : GetIpAddresses())
            result->Add(eventDispatcher, address, port, versions);

        return result;
    }

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Listen(DatagramExchangeObserver& observer, IPVersions versions)
    {
        auto result = infra::MakeSharedOnHeap<DatagramExchangeMultiple>(observer, eventDispatcher);
        eventDispatcher.RegisterDatagramMultiple(result);

        for (auto address : GetIpAddresses())
            result->Add(eventDispatcher, address, versions);

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
}
