#include "services/network_instantiations/ConnectionBsd.hpp"
#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "services/network_instantiations/EventDispatcherWithNetworkBsd.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <optional>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
    template<class SockAddr>
    const sockaddr* AsSockAddr(const SockAddr& address)
    {
        return static_cast<const sockaddr*>(static_cast<const void*>(&address));
    }

    template<class SockAddr>
    sockaddr* AsSockAddr(SockAddr& address)
    {
        return static_cast<sockaddr*>(static_cast<void*>(&address));
    }

    template<class SockAddr>
    int BindSocket(int fileDescriptor, const SockAddr& address)
    {
        return bind(fileDescriptor, AsSockAddr(address), sizeof(address));
    }

    template<class SockAddr>
    int ConnectSocket(int fileDescriptor, const SockAddr& address)
    {
        return connect(fileDescriptor, AsSockAddr(address), sizeof(address));
    }

    void GetPeerName(int fileDescriptor, sockaddr_storage& address, socklen_t& addressLength)
    {
        if (getpeername(fileDescriptor, AsSockAddr(address), &addressLength) == -1)
            std::abort();
    }

    void SetNonBlocking(int fileDescriptor)
    {
        auto status_flags = fcntl(fileDescriptor, F_GETFL, 0);
        if (status_flags == -1)
            std::abort();
        if (fcntl(fileDescriptor, F_SETFL, status_flags | O_NONBLOCK) == -1)
            std::abort();
    }

    void EnableKeepAlive(int fileDescriptor)
    {
        int enabled = 1;
        setsockopt(fileDescriptor, SOL_SOCKET, SO_KEEPALIVE, &enabled, sizeof(enabled));

#ifdef __linux__
        int idleSeconds = 10;
        int intervalSeconds = 3;
        int probeCount = 3;

        setsockopt(fileDescriptor, IPPROTO_TCP, TCP_KEEPIDLE, &idleSeconds, sizeof(idleSeconds));
        setsockopt(fileDescriptor, IPPROTO_TCP, TCP_KEEPINTVL, &intervalSeconds, sizeof(intervalSeconds));
        setsockopt(fileDescriptor, IPPROTO_TCP, TCP_KEEPCNT, &probeCount, sizeof(probeCount));
#endif
    }

    services::IPv6Address ToIpv6Address(const in6_addr& address)
    {
        services::IPv6Address ipv6Address{};
        for (std::size_t i = 0; i != ipv6Address.size(); ++i)
        {
            uint16_t value;
            std::memcpy(&value, &address.s6_addr[i * 2], sizeof(value));
            ipv6Address[i] = ntohs(value);
        }

        return ipv6Address;
    }

    in6_addr ToIn6Addr(const services::IPv6Address& address)
    {
        in6_addr in6Address{};
        for (std::size_t i = 0; i != address.size(); ++i)
        {
            const auto value = htons(address[i]);
            std::memcpy(&in6Address.s6_addr[i * 2], &value, sizeof(value));
        }

        return in6Address;
    }

    std::optional<services::IPv4Address> ToIpv4AddressWhenV4Mapped(const in6_addr& address)
    {
        if (!IN6_IS_ADDR_V4MAPPED(&address))
            return std::nullopt;

        const std::array<uint8_t, 4> ipv4Bytes{ address.s6_addr[12], address.s6_addr[13], address.s6_addr[14], address.s6_addr[15] };
        return services::IPv4Address{ ipv4Bytes[0], ipv4Bytes[1], ipv4Bytes[2], ipv4Bytes[3] };
    }
}

namespace services
{
    ConnectionBsd::ConnectionBsd(EventDispatcherWithNetwork& network, int socket)
        : network(network)
        , socket(socket)
        , streamReader([this]()
              {
                  keepAliveForReader = nullptr;
              })
    {
        SetNonBlocking(socket);
        EnableKeepAlive(socket);
    }

    ConnectionBsd::~ConnectionBsd()
    {
        if (Connected())
        {
            int result = close(socket);
            if (result == -1)
                std::abort();
        }
    }

    void ConnectionBsd::RequestSendStream(std::size_t sendSize)
    {
        assert(requestedSendSize == 0);
        assert(sendSize != 0 && sendSize <= MaxSendStreamSize());

        requestedSendSize = sendSize;
        TryAllocateSendStream();
    }

    std::size_t ConnectionBsd::MaxSendStreamSize() const
    {
        return sendBuffer.max_size();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionBsd::ReceiveStream()
    {
        keepAliveForReader = SharedFromThis();
        return streamReader.Emplace(*this);
    }

    void ConnectionBsd::AckReceived()
    {
        streamReader->ConsumeRead();
    }

    void ConnectionBsd::CloseAndDestroy()
    {
        AbortAndDestroy();
    }

    void ConnectionBsd::AbortAndDestroy()
    {
        int result = close(socket);
        assert(result != -1);
        socket = 0;
        ResetOwnership();
    }

    IPAddress ConnectionBsd::Address() const
    {
        sockaddr_storage address{};
        socklen_t addressLength = sizeof(address);
        GetPeerName(socket, address, addressLength);

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

    void ConnectionBsd::SetObserver(infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        SetSelfOwnership(connectionObserver);
        network.RegisterConnection(SharedFromThis());
        Attach(connectionObserver);
    }

    bool ConnectionBsd::Connected() const
    {
        return socket != 0;
    }

    bool ConnectionBsd::SendBufferEmpty() const
    {
        return sendBuffer.empty();
    }

    void ConnectionBsd::Receive()
    {
        while (!receiveBuffer.full())
        {
            std::array<uint8_t, 2048> buffer;
            int received = recv(socket, reinterpret_cast<char*>(buffer.data()), receiveBuffer.max_size() - receiveBuffer.size(), 0);
            if (received == -1)
            {
                if (errno != EWOULDBLOCK)
                    AbortAndDestroy();
                return;
            }
            else if (received != 0)
            {
                receiveBuffer.insert(receiveBuffer.end(), buffer.data(), buffer.data() + received);

                infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionBsd>& object)
                    {
                        object->Observer().DataReceived();
                    },
                    SharedFromThis());
            }
            else
            {
                CloseAndDestroy();
                return;
            }
        }
    }

    void ConnectionBsd::Send()
    {
        long sent = 0;

        do
        {
            std::vector<char> tmpBuffer(sendBuffer.begin(), sendBuffer.end());
            sent = send(socket, tmpBuffer.data(), tmpBuffer.size(), 0);

            if (sent == -1)
            {
                if (errno != EWOULDBLOCK)
                    ResetOwnership();
                return;
            }

            sendBuffer.erase(sendBuffer.begin(), sendBuffer.begin() + sent);
        } while (sent != 0 && !sendBuffer.empty());

        if (requestedSendSize != 0)
            TryAllocateSendStream();
    }

    void ConnectionBsd::TrySend()
    {
        if (trySend)
        {
            trySend = false;
            Send();
        }
    }

    void ConnectionBsd::SetSelfOwnership(const infra::SharedPtr<ConnectionObserver>& observer)
    {
        self = SharedFromThis();
    }

    void ConnectionBsd::ResetOwnership()
    {
        if (IsAttached())
            Detach();
        self = nullptr;
    }

    void ConnectionBsd::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (sendBuffer.max_size() - sendBuffer.size() >= requestedSendSize)
        {
            auto size = requestedSendSize;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([size](const infra::SharedPtr<ConnectionBsd>& object)
                {
                    infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object, size);
                    object->Observer().SendStreamAvailable(std::move(writer));
                },
                SharedFromThis());

            requestedSendSize = 0;
        }
    }

    ConnectionBsd::StreamWriterBsd::StreamWriterBsd(ConnectionBsd& connection, std::size_t size)
        : std::vector<uint8_t>(size, 0)
        , infra::ByteOutputStreamWriter(infra::MakeRange(*this))
        , connection(connection)
    {}

    ConnectionBsd::StreamWriterBsd::~StreamWriterBsd()
    {
        connection.sendBuffer.insert(connection.sendBuffer.end(), Processed().begin(), Processed().end());
        connection.trySend = true;
    }

    ConnectionBsd::StreamReaderBsd::StreamReaderBsd(ConnectionBsd& connection)
        : infra::BoundedDequeInputStreamReader(connection.receiveBuffer)
        , connection(connection)
    {}

    void ConnectionBsd::StreamReaderBsd::ConsumeRead()
    {
        connection.receiveBuffer.erase(connection.receiveBuffer.begin(), connection.receiveBuffer.begin() + ConstructSaveMarker());
        Rewind(0);
    }

    ListenerBsd::ListenerBsd(EventDispatcherWithNetwork& network, uint16_t port, services::ServerConnectionObserverFactory& factory, IPVersions versions)
        : network(network)
        , factory(factory)
    {
        const auto family = versions == IPVersions::ipv4 ? AF_INET : AF_INET6;

        listenSocket = socket(family, SOCK_STREAM, IPPROTO_TCP);
        assert(listenSocket != -1);

        int reuseAddress = 1;
        setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress));

        if (family == AF_INET6)
        {
            int v6Only = versions == IPVersions::ipv6 ? 1 : 0;
            setsockopt(listenSocket, IPPROTO_IPV6, IPV6_V6ONLY, &v6Only, sizeof(v6Only));
        }

        if (family == AF_INET)
        {
            sockaddr_in address = {};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_ANY);
            address.sin_port = htons(port);
            if (BindSocket(listenSocket, address) == -1)
                std::abort();
        }
        else
        {
            sockaddr_in6 address = {};
            address.sin6_family = AF_INET6;
            address.sin6_addr = in6addr_any;
            address.sin6_port = htons(port);
            if (BindSocket(listenSocket, address) == -1)
                std::abort();
        }

        if (listen(listenSocket, 1) == -1)
            std::abort();

        SetNonBlocking(listenSocket);
        network.RegisterListener(*this);
    }

    ListenerBsd::~ListenerBsd()
    {
        network.DeregisterListener(*this);
    }

    void ListenerBsd::Accept()
    {
        auto acceptedSocket = accept(listenSocket, nullptr, nullptr);
        assert(acceptedSocket != -1);

        infra::SharedPtr<ConnectionBsd> connection = infra::MakeSharedOnHeap<ConnectionBsd>(network, acceptedSocket);
        factory.ConnectionAccepted([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                if (connectionObserver)
                    connection->SetObserver(connectionObserver);
            },
            connection->Address());
    }

    ConnectorBsd::ConnectorBsd(EventDispatcherWithNetwork& network, services::ClientConnectionObserverFactory& factory)
        : network(network)
        , factory(factory)
    {
        const auto address = factory.Address();

        if (std::holds_alternative<services::IPv4Address>(address))
        {
            auto ipv4Address = std::get<services::IPv4Address>(address);

            connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            assert(connectSocket != -1);
            SetNonBlocking(connectSocket);

            sockaddr_in saddress = {};
            saddress.sin_family = AF_INET;
            saddress.sin_addr.s_addr = htonl(services::ConvertToUint32(ipv4Address));
            saddress.sin_port = htons(factory.Port());
            if (ConnectSocket(connectSocket, saddress) == -1 && errno != EWOULDBLOCK && errno != EINPROGRESS)
                std::abort();

            return;
        }

        const auto& ipv6Address = std::get<services::IPv6Address>(address);

        connectSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        assert(connectSocket != -1);
        SetNonBlocking(connectSocket);

        sockaddr_in6 saddress = {};
        saddress.sin6_family = AF_INET6;
        saddress.sin6_addr = ToIn6Addr(ipv6Address);
        saddress.sin6_port = htons(factory.Port());
        if (ConnectSocket(connectSocket, saddress) == -1 && errno != EWOULDBLOCK && errno != EINPROGRESS)
            std::abort();
    }

    ConnectorBsd::~ConnectorBsd()
    {
        if (connectSocket != -1)
            close(connectSocket);
    }

    void ConnectorBsd::Connected()
    {
        infra::SharedPtr<ConnectionBsd> connection = infra::MakeSharedOnHeap<ConnectionBsd>(network, connectSocket);
        connectSocket = -1;
        factory.ConnectionEstablished([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                if (connectionObserver)
                    connection->SetObserver(connectionObserver);
            });

        network.DeregisterConnector(*this);
    }

    void ConnectorBsd::Failed()
    {
        factory.ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::refused);

        network.DeregisterConnector(*this);
    }
}
