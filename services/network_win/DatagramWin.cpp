#include "infra/stream/ByteInputStream.hpp"
#include "services/network_win/DatagramWin.hpp"
#include "services/network_win/EventDispatcherWithNetwork.hpp"
#include <ws2tcpip.h>

namespace services
{
    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, uint16_t port, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, port });
    }

    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, 0 });
    }

    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, UdpSocket remote, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, 0 });
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, uint16_t localPort, UdpSocket remote, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(Udpv4Socket{ IPv4Address{}, localPort });
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, IPAddress localAddress, UdpSocket remote, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(MakeUdpSocket(localAddress, 0));
        BindRemote(remote);
    }

    DatagramWin::DatagramWin(EventDispatcherWithNetwork& network, UdpSocket local, UdpSocket remote, DatagramExchangeObserver& observer)
        : network(network)
    {
        observer.Attach(*this);
        InitSocket();
        BindLocal(local);
        BindRemote(remote);
    }

    DatagramWin::~DatagramWin()
    {
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
            infra::ByteInputStreamReader reader(infra::MakeRange(receiveBuffer));

            auto from = Udpv4Socket{ services::ConvertFromUint32(htonl(fromAddress.sin_addr.s_addr)), fromAddress.sin_port };

            GetObserver().DataReceived(reader, from);
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
        UpdateEventFlags();
    }

    void DatagramWin::TrySend()
    {
        if (trySend)
        {
            trySend = false;
            Send();
        }
    }

    void DatagramWin::UpdateEventFlags()
    {
        int result = WSAEventSelect(socket, event, FD_READ | (sendBuffer ? FD_WRITE : 0));
        assert(result == 0);
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
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(services::ConvertToUint32(local.Get<Udpv4Socket>().first));
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
        if (!sendBuffer)
        {
            sendBuffer.Emplace(requestedSendSize, 0);
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
    }

    DatagramExchangeMultiple::DatagramExchangeMultiple(DatagramExchangeObserver& observer)
    {
        observer.Attach(*this);
    }

    DatagramExchangeMultiple::~DatagramExchangeMultiple()
    {
        GetObserver().Detach();
    }

    void DatagramExchangeMultiple::Add(DatagramFactoryWithLocalIpBinding& factory, IPAddress local, UdpSocket remote)
    {
        observers.push_back(infra::MakeSharedOnHeap<Observer>(*this, factory, local, remote));
    }

    void DatagramExchangeMultiple::Add(DatagramFactoryWithLocalIpBinding& factory, UdpSocket local, UdpSocket remote)
    {
        observers.push_back(infra::MakeSharedOnHeap<Observer>(*this, factory, local, remote));
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

    void DatagramExchangeMultiple::Observer::DataReceived(infra::StreamReaderWithRewinding& reader, UdpSocket from)
    {
        parent.GetObserver().DataReceived(reader, from);
    }

    void DatagramExchangeMultiple::Observer::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        parent.writers.emplace_back(std::move(writer));

        if (parent.writers.size() == parent.observers.size())
        {
            parent.GetObserver().SendStreamAvailable(parent.multipleWriter.Emplace(parent.writers));
            parent.writers.clear();
        }
    }

    DatagramExchangeMultiple::MultipleWriter::MultipleWriter(const std::vector<infra::SharedPtr<infra::StreamWriter>>& writers)
        : writers(writers)
    {}

    void DatagramExchangeMultiple::MultipleWriter::Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy)
    {
        for (auto& writer : writers)
        {
            ++const_cast<uint8_t&>(range[0]);
            writer->Insert(range, errorPolicy);
        }
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

    UdpOnAllInterfaces::UdpOnAllInterfaces(DatagramFactoryWithLocalIpBinding& datagramFactory)
        : datagramFactory(datagramFactory)
    {}

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Listen(DatagramExchangeObserver& observer, uint16_t port, IPVersions versions)
    {
        return datagramFactory.Listen(observer, port, versions);
    }

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Listen(DatagramExchangeObserver& observer, IPVersions versions)
    {
        return datagramFactory.Listen(observer, versions);
    }

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Connect(DatagramExchangeObserver& observer, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramExchangeMultiple>(observer);

        for (auto address : GetIpAddresses())
            result->Add(datagramFactory, address, remote);

        return result;
    }

    infra::SharedPtr<DatagramExchange> UdpOnAllInterfaces::Connect(DatagramExchangeObserver& observer, uint16_t localPort, UdpSocket remote)
    {
        auto result = infra::MakeSharedOnHeap<DatagramExchangeMultiple>(observer);

        for (auto address : GetIpAddresses())
            result->Add(datagramFactory, MakeUdpSocket(address, localPort), remote);

        return result;
    }

    std::vector<IPv4Address> UdpOnAllInterfaces::GetIpAddresses()
    {
        ULONG size(0);
        auto result = GetAdaptersInfo(nullptr, &size);
        assert(result == ERROR_BUFFER_OVERFLOW);
        auto adapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(size));
        result = GetAdaptersInfo(adapterInfo, &size);
        assert(result == NO_ERROR);

        std::vector<IPv4Address> addresses;

        for (; adapterInfo != nullptr; adapterInfo = adapterInfo->Next)
        {
            auto address = Translate(adapterInfo->IpAddressList.IpAddress);
            if (address != IPv4Address())
                addresses.push_back(address);
        }

        return addresses;
    }

    IPv4Address UdpOnAllInterfaces::Translate(const IP_ADDRESS_STRING& address)
    {
        std::string terminatedHostname(&address.String[0], &address.String[sizeof(IP_ADDRESS_STRING::String)]);
        terminatedHostname.push_back('\0');

        addrinfo* entry;
        int res = getaddrinfo(terminatedHostname.c_str(), nullptr, nullptr, &entry);
        if (res == 0)
            for (; entry != nullptr; entry = entry->ai_next)
                if (entry->ai_family == AF_INET)
                {
                    sockaddr_in* address = reinterpret_cast<sockaddr_in*>(entry->ai_addr);
                    return IPv4Address{ address->sin_addr.s_net, address->sin_addr.s_host, address->sin_addr.s_lh, address->sin_addr.s_impno };
                }

        return IPv4Address();
    }
}
