#include "services/network_win/ConnectionWin.hpp"
#include "services/network_win/EventDispatcherWithNetwork.hpp"

namespace services
{
    ConnectionWin::ConnectionWin(EventDispatcherWithNetwork& network, SOCKET socket)
        : network(network)
        , socket(socket)
    {
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
                DWORD error = GetLastError();
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
        int result = closesocket(socket);
        assert(result != SOCKET_ERROR);
        socket = 0;
        ResetOwnership();
    }

    IPv4Address ConnectionWin::Ipv4Address() const
    {
        sockaddr_in address{};
        int addressLength = sizeof(address);
        getpeername(socket, reinterpret_cast<SOCKADDR*>(&address), &addressLength);

        return services::ConvertFromUint32(htonl(address.sin_addr.s_addr));
    }

    void ConnectionWin::SetObserver(infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        SetSelfOwnership(connectionObserver);
        network.RegisterConnection(SharedFromThis());
        Attach(connectionObserver);
    }

    void ConnectionWin::Receive()
    {
        while (!receiveBuffer.full())
        {
            std::array<uint8_t, 2048> buffer;
            int received = recv(socket, reinterpret_cast<char*>(buffer.data()), receiveBuffer.max_size() - receiveBuffer.size(), 0);
            if (received == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                    ResetOwnership();
                return;
            }
            else if (received != 0)
            {
                receiveBuffer.insert(receiveBuffer.end(), buffer.data(), buffer.data() + received);

                infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionWin>& object)
                {
                    object->Observer().DataReceived();
                }, SharedFromThis());
            }
            else
            {
                ResetOwnership();
                return;
            }
        }
    }

    void ConnectionWin::Send()
    {
        int sent = 0;

        do
        {
            UpdateEventFlags();     // If there is something to send, update the flags before calling send, because FD_SEND is an edge-triggered event.
            sent = send(socket, reinterpret_cast<char*>(sendBuffer.contiguous_range(sendBuffer.begin()).begin())
                , sendBuffer.contiguous_range(sendBuffer.begin()).size(), 0);

            if (sent == SOCKET_ERROR)
            {
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                    ResetOwnership();
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
                infra::SharedPtr<infra::StreamWriter> writer = object->streamWriter.Emplace(*object, size);
                object->Observer().SendStreamAvailable(std::move(writer));
            }, SharedFromThis());

            requestedSendSize = 0;
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

    ListenerWin::ListenerWin(EventDispatcherWithNetwork& network, uint16_t port, services::ServerConnectionObserverFactory& factory)
        : network(network)
        , factory(factory)
    {
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        assert(listenSocket != INVALID_SOCKET);
        int result = WSAEventSelect(listenSocket, event, FD_ACCEPT);
        assert(result == 0);

        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
        if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&address), sizeof(SOCKADDR)) == SOCKET_ERROR)
            std::abort();

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
        }, connection->Ipv4Address());
    }

    ConnectorWin::ConnectorWin(EventDispatcherWithNetwork& network, services::ClientConnectionObserverFactory& factory)
        : network(network)
        , factory(factory)
    {
        auto address = factory.Address().Get<services::IPv4Address>();

        connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        assert(connectSocket != INVALID_SOCKET);
        int result = WSAEventSelect(connectSocket, event, FD_CONNECT);
        assert(result == 0);

        ULONG nonBlock = 1;
        if (ioctlsocket(connectSocket, FIONBIO, &nonBlock) == SOCKET_ERROR)
            std::abort();

        sockaddr_in saddress = {};
        saddress.sin_family = AF_INET;
        saddress.sin_addr.s_addr = htonl(services::ConvertToUint32(address));
        saddress.sin_port = htons(factory.Port());
        if (connect(connectSocket, reinterpret_cast<sockaddr*>(&saddress), sizeof(saddress)) == SOCKET_ERROR)
        {
            if (GetLastError() != WSAEWOULDBLOCK)
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
        infra::WeakPtr<ConnectorWin> self = SharedFromThis();
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
        infra::WeakPtr<ConnectorWin> self = SharedFromThis();
        factory.ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::refused);

        network.DeregisterConnector(*this);
    }
}
