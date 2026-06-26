#include "services/network_instantiations/ConnectionWin.hpp"
#include "services/network_instantiations/EventDispatcherWithNetworkWin.hpp"
#include <cstring>
#include <optional>
#include <ws2tcpip.h>

namespace
{
    void EnableKeepAlive(SOCKET socket)
    {
        int enabled = 1;
        setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&enabled), sizeof(enabled));
    }

    services::IPv6Address ToIpv6Address(const IN6_ADDR& address)
    {
        services::IPv6Address ipv6Address{};
        for (std::size_t i = 0; i != ipv6Address.size(); ++i)
        {
            uint16_t value;
            std::memcpy(&value, &address.u.Byte[i * 2], sizeof(value));
            ipv6Address[i] = ntohs(value);
        }

        return ipv6Address;
    }

    IN6_ADDR ToIn6Addr(const services::IPv6Address& address)
    {
        IN6_ADDR in6Address{};
        for (std::size_t i = 0; i != address.size(); ++i)
        {
            const auto value = htons(address[i]);
            std::memcpy(&in6Address.u.Byte[i * 2], &value, sizeof(value));
        }

        return in6Address;
    }

    std::optional<services::IPv4Address> ToIpv4AddressWhenV4Mapped(const IN6_ADDR& address)
    {
        if (!IN6_IS_ADDR_V4MAPPED(&address))
            return std::nullopt;

        return services::IPv4Address{ address.u.Byte[12], address.u.Byte[13], address.u.Byte[14], address.u.Byte[15] };
    }
}

namespace services
{
    ConnectionWin::ConnectionWin(EventDispatcherWithNetwork& network, SOCKET socket)
        : network(network)
        , socket(socket)
        , streamReader([this]()
              {
                  keepAliveForReader = nullptr;
              })
    {
        EnableKeepAlive(socket);
        UpdateEventFlags();
    }

    ConnectionWin::~ConnectionWin()
    {
        if (socket != 0)
        {
            BOOL result = WSACloseEvent(event);
            assert(result == TRUE);
            result = closesocket(socket);
            if (result == SOCKET_ERROR)
            {
                DWORD error = WSAGetLastError();
                std::abort();
            }
        }
    }

    void ConnectionWin::RequestSendStream(std::size_t sendSize)
    {
        assert(requestedSendSize == 0);
        assert(sendSize != 0 && sendSize <= MaxSendStreamSize());
        requestedSendSize = sendSize;
        TryAllocateSendStream();
    }

    std::size_t ConnectionWin::MaxSendStreamSize() const
    {
        return sendBuffer.max_size();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionWin::ReceiveStream()
    {
        keepAliveForReader = SharedFromThis();
        return streamReader.Emplace(*this);
    }

    void ConnectionWin::AckReceived()
    {
        streamReader->ConsumeRead();
    }

    void ConnectionWin::CloseAndDestroy()
    {
        AbortAndDestroy();
    }

    void ConnectionWin::AbortAndDestroy()
    {
        if (socket == 0)
            return;

        int result = closesocket(socket);
        assert(result != SOCKET_ERROR);
        socket = 0;
        ResetOwnership();
    }

    IPAddress ConnectionWin::Address() const
    {
        sockaddr_storage address{};
        int addressLength = sizeof(address);
        if (getpeername(socket, reinterpret_cast<SOCKADDR*>(&address), &addressLength) == SOCKET_ERROR)
            std::abort();

        if (address.ss_family == AF_INET)
        {
            sockaddr_in ipv4Address{};
            std::memcpy(&ipv4Address, &address, sizeof(ipv4Address));
            return services::ConvertFromUint32(htonl(ipv4Address.sin_addr.s_addr));
        }

        sockaddr_in6 ipv6Address{};
        std::memcpy(&ipv6Address, &address, sizeof(ipv6Address));
        if (auto ipv4Address = ToIpv4AddressWhenV4Mapped(ipv6Address.sin6_addr); ipv4Address)
            return *ipv4Address;

        return ToIpv6Address(ipv6Address.sin6_addr);
    }

    void ConnectionWin::SetObserver(infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        SetSelfOwnership(connectionObserver);
        network.RegisterConnection(SharedFromThis());
        Attach(connectionObserver);
    }

    bool ConnectionWin::Connected() const
    {
        return socket != 0;
    }

    void ConnectionWin::Receive()
    {
        if (socket == 0)
            return;

        while (!receiveBuffer.full())
        {
            std::array<uint8_t, 2048> buffer;
            int received = recv(socket, reinterpret_cast<char*>(buffer.data()), receiveBuffer.max_size() - receiveBuffer.size(), 0);
            if (received == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                    AbortAndDestroy();
                return;
            }
            else if (received != 0)
            {
                receiveBuffer.insert(receiveBuffer.end(), buffer.data(), buffer.data() + received);

                ScheduleDataReceivedIfNeeded();
            }
            else
            {
                closePending = true;
                FinalizeCloseIfReady();
                return;
            }
        }
    }

    void ConnectionWin::Send()
    {
        if (socket == 0)
            return;

        int sent = 0;

        do
        {
            UpdateEventFlags(); // If there is something to send, update the flags before calling send, because FD_SEND is an edge-triggered event.
            std::vector<char> tmpBuffer(sendBuffer.begin(), sendBuffer.end());
            sent = send(socket, tmpBuffer.data(), tmpBuffer.size(), 0);

            if (sent == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                    AbortAndDestroy();
                return;
            }

            UpdateEventFlags();

            sendBuffer.erase(sendBuffer.begin(), sendBuffer.begin() + sent);
        } while (sent != 0 && !sendBuffer.empty());

        if (requestedSendSize != 0)
            TryAllocateSendStream();
    }

    void ConnectionWin::TrySend()
    {
        if (trySend)
        {
            trySend = false;
            Send();
        }
    }

    void ConnectionWin::UpdateEventFlags()
    {
        int result = WSAEventSelect(socket, event, (!receiveBuffer.full() ? FD_READ : 0) | (!sendBuffer.empty() ? FD_WRITE : 0) | FD_CLOSE);
        assert(result == 0);
    }

    void ConnectionWin::SetSelfOwnership(const infra::SharedPtr<ConnectionObserver>& observer)
    {
        self = SharedFromThis();
    }

    void ConnectionWin::ResetOwnership()
    {
        if (IsAttached())
            Detach();
        dataReceivedScheduled = false;
        closePending = false;
        self = nullptr;
    }

    void ConnectionWin::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (sendBuffer.max_size() - sendBuffer.size() >= requestedSendSize)
        {
            auto size = requestedSendSize;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([size](const infra::SharedPtr<ConnectionWin>& object)
                {
                    if (object->IsAttached())
                    {
                        infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object, size);
                        object->Observer().SendStreamAvailable(std::move(writer));
                    }
                },
                SharedFromThis());

            requestedSendSize = 0;
        }
    }

    void ConnectionWin::ScheduleDataReceivedIfNeeded()
    {
        if (dataReceivedScheduled)
            return;

        dataReceivedScheduled = true;
        infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionWin>& object)
            {
                auto bufferedBeforeCallback = object->receiveBuffer.size();

                object->dataReceivedScheduled = false;
                if (object->IsAttached())
                {
                    object->Observer().DataReceived();

                    auto bufferedAfterCallback = object->receiveBuffer.size();
                    if (object->closePending && bufferedAfterCallback != 0 && bufferedAfterCallback < bufferedBeforeCallback)
                    {
                        object->ScheduleDataReceivedIfNeeded();
                        return;
                    }
                }

                object->FinalizeCloseIfReady();
            },
            SharedFromThis());
    }

    void ConnectionWin::FinalizeCloseIfReady()
    {
        if (closePending && !dataReceivedScheduled)
        {
            CloseAndDestroy();
        }
    }

    ConnectionWin::StreamWriterWin::StreamWriterWin(ConnectionWin& connection, std::size_t size)
        : std::vector<uint8_t>(size, 0)
        , infra::ByteOutputStreamWriter(infra::MakeRange(*this))
        , connection(connection)
    {}

    ConnectionWin::StreamWriterWin::~StreamWriterWin()
    {
        connection.sendBuffer.insert(connection.sendBuffer.end(), Processed().begin(), Processed().end());
        connection.trySend = true;
    }

    ConnectionWin::StreamReaderWin::StreamReaderWin(ConnectionWin& connection)
        : infra::BoundedDequeInputStreamReader(connection.receiveBuffer)
        , connection(connection)
    {}

    void ConnectionWin::StreamReaderWin::ConsumeRead()
    {
        connection.receiveBuffer.erase(connection.receiveBuffer.begin(), connection.receiveBuffer.begin() + ConstructSaveMarker());
        Rewind(0);
    }

    ListenerWin::ListenerWin(EventDispatcherWithNetwork& network, uint16_t port, services::ServerConnectionObserverFactory& factory, IPVersions versions)
        : network(network)
        , factory(factory)
    {
        const auto family = versions == IPVersions::ipv4 ? AF_INET : AF_INET6;

        listenSocket = socket(family, SOCK_STREAM, IPPROTO_TCP);
        assert(listenSocket != INVALID_SOCKET);
        int result = WSAEventSelect(listenSocket, event, FD_ACCEPT);
        assert(result == 0);

        int exclusiveAddressUse = 1;
        if (setsockopt(listenSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char*>(&exclusiveAddressUse), sizeof(exclusiveAddressUse)) == SOCKET_ERROR)
            std::abort();

        if (family == AF_INET6)
        {
            int v6Only = versions == IPVersions::ipv6 ? 1 : 0;
            if (setsockopt(listenSocket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&v6Only), sizeof(v6Only)) == SOCKET_ERROR)
                std::abort();
        }

        if (family == AF_INET)
        {
            sockaddr_in address = {};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_ANY);
            address.sin_port = htons(port);
            if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&address), sizeof(address)) == SOCKET_ERROR)
                std::abort();
        }
        else
        {
            sockaddr_in6 address = {};
            address.sin6_family = AF_INET6;
            address.sin6_addr = in6addr_any;
            address.sin6_port = htons(port);
            if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&address), sizeof(address)) == SOCKET_ERROR)
                std::abort();
        }

        if (listen(listenSocket, 1) == SOCKET_ERROR)
            std::abort();

        ULONG nonBlock = 1;
        if (ioctlsocket(listenSocket, FIONBIO, &nonBlock) == SOCKET_ERROR)
            std::abort();

        network.RegisterListener(*this);
    }

    ListenerWin::~ListenerWin()
    {
        BOOL result = WSACloseEvent(event);
        assert(result == TRUE);
        network.DeregisterListener(*this);
    }

    void ListenerWin::Accept()
    {
        SOCKET acceptedSocket = accept(listenSocket, NULL, NULL);
        assert(acceptedSocket != INVALID_SOCKET);

        infra::SharedPtr<ConnectionWin> connection = infra::MakeSharedOnHeap<ConnectionWin>(network, acceptedSocket);
        factory.ConnectionAccepted([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                if (connectionObserver)
                    connection->SetObserver(connectionObserver);
            },
            connection->Address());
    }

    ConnectorWin::ConnectorWin(EventDispatcherWithNetwork& network, services::ClientConnectionObserverFactory& factory)
        : network(network)
        , factory(factory)
    {
        const auto address = factory.Address();

        if (std::holds_alternative<services::IPv4Address>(address))
        {
            auto ipv4Address = std::get<services::IPv4Address>(address);

            connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            assert(connectSocket != INVALID_SOCKET);
            int result = WSAEventSelect(connectSocket, event, FD_CONNECT);
            assert(result == 0);

            ULONG nonBlock = 1;
            if (ioctlsocket(connectSocket, FIONBIO, &nonBlock) == SOCKET_ERROR)
                std::abort();

            sockaddr_in saddress = {};
            saddress.sin_family = AF_INET;
            saddress.sin_addr.s_addr = htonl(services::ConvertToUint32(ipv4Address));
            saddress.sin_port = htons(factory.Port());
            if (connect(connectSocket, reinterpret_cast<sockaddr*>(&saddress), sizeof(saddress)) == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                    std::abort();
            }

            return;
        }

        const auto& ipv6Address = std::get<services::IPv6Address>(address);

        connectSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        assert(connectSocket != INVALID_SOCKET);
        int result = WSAEventSelect(connectSocket, event, FD_CONNECT);
        assert(result == 0);

        ULONG nonBlock = 1;
        if (ioctlsocket(connectSocket, FIONBIO, &nonBlock) == SOCKET_ERROR)
            std::abort();

        sockaddr_in6 saddress = {};
        saddress.sin6_family = AF_INET6;
        saddress.sin6_addr = ToIn6Addr(ipv6Address);
        saddress.sin6_port = htons(factory.Port());
        if (connect(connectSocket, reinterpret_cast<sockaddr*>(&saddress), sizeof(saddress)) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                std::abort();
        }
    }

    ConnectorWin::~ConnectorWin()
    {
        BOOL result = WSACloseEvent(event);
        assert(result == TRUE);
    }

    void ConnectorWin::Connected()
    {
        infra::SharedPtr<ConnectionWin> connection = infra::MakeSharedOnHeap<ConnectionWin>(network, connectSocket);
        factory.ConnectionEstablished([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                if (connectionObserver)
                    connection->SetObserver(connectionObserver);
            });

        network.DeregisterConnector(*this);
    }

    void ConnectorWin::Failed()
    {
        factory.ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::refused);

        network.DeregisterConnector(*this);
    }
}
