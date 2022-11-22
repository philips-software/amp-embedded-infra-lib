#include "services/network_bsd/ConnectionBsd.hpp"
#include "services/network_bsd/EventDispatcherWithNetwork.hpp"
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
    void SetNonBlocking(int fileDescriptor)
    {
        if (fcntl(fileDescriptor, F_SETFL, fcntl(fileDescriptor, F_GETFL, 0) | O_NONBLOCK) == -1)
            std::abort();
    }
}

namespace services
{
    ConnectionBsd::ConnectionBsd(EventDispatcherWithNetwork& network, int socket)
        : network(network)
        , socket(socket)
    {}

    ConnectionBsd::~ConnectionBsd()
    {
        if (socket != 0)
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

    IPv4Address ConnectionBsd::Ipv4Address() const
    {
        sockaddr_in address{};
        socklen_t addressLength = sizeof(address);
        getpeername(socket, reinterpret_cast<sockaddr*>(&address), &addressLength);

        return services::ConvertFromUint32(htonl(address.sin_addr.s_addr));
    }

    void ConnectionBsd::SetObserver(infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        SetSelfOwnership(connectionObserver);
        network.RegisterConnection(SharedFromThis());
        Attach(connectionObserver);
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
                    ResetOwnership();
                return;
            }
            else if (received != 0)
            {
                receiveBuffer.insert(receiveBuffer.end(), buffer.data(), buffer.data() + received);

                infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionBsd>& object)
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

    void ConnectionBsd::Send()
    {
        int sent = 0;

        do
        {
            sent = send(socket, reinterpret_cast<char*>(sendBuffer.contiguous_range(sendBuffer.begin()).begin())
                , sendBuffer.contiguous_range(sendBuffer.begin()).size(), 0);

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
            }, SharedFromThis());

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

    ListenerBsd::ListenerBsd(EventDispatcherWithNetwork& network, uint16_t port, services::ServerConnectionObserverFactory& factory)
        : network(network)
        , factory(factory)
    {
        listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        assert(listenSocket != -1);

        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
        if (bind(listenSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1)
            std::abort();

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
        auto acceptedSocket = accept(listenSocket, NULL, NULL);
        assert(acceptedSocket != -1);

        infra::SharedPtr<ConnectionBsd> connection = infra::MakeSharedOnHeap<ConnectionBsd>(network, acceptedSocket);
        factory.ConnectionAccepted([connection](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
        {
            if (connectionObserver)
                connection->SetObserver(connectionObserver);
        }, connection->Ipv4Address());
    }

    ConnectorBsd::ConnectorBsd(EventDispatcherWithNetwork& network, services::ClientConnectionObserverFactory& factory)
        : network(network)
        , factory(factory)
    {
        auto address = factory.Address().Get<services::IPv4Address>();

        connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        assert(connectSocket != -1);
        SetNonBlocking(connectSocket);

        sockaddr_in saddress = {};
        saddress.sin_family = AF_INET;
        saddress.sin_addr.s_addr = htonl(services::ConvertToUint32(address));
        saddress.sin_port = htons(factory.Port());
        if (connect(connectSocket, reinterpret_cast<sockaddr*>(&saddress), sizeof(saddress)) == -1)
        {
            if (errno != EWOULDBLOCK)
                std::abort();
        }
    }

    ConnectorBsd::~ConnectorBsd()
    {
        close(connectSocket);
    }

    void ConnectorBsd::Connected()
    {
        infra::SharedPtr<ConnectionBsd> connection = infra::MakeSharedOnHeap<ConnectionBsd>(network, connectSocket);
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
