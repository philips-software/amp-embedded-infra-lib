#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "services/network/test_doubles/ConnectionLoopBack.hpp"

namespace services
{
    ConnectionLoopBackPeer::ConnectionLoopBackPeer(ConnectionLoopBackPeer& peer, ConnectionLoopBack& loopBack)
        : peer(peer)
        , loopBack(loopBack)
    {}

    void ConnectionLoopBackPeer::RequestSendStream(std::size_t sendSize)
    {
        assert(requestedSendSize == 0);
        assert(streamWriter.Allocatable());
        requestedSendSize = sendSize;
        TryAllocateSendStream();
    }

    std::size_t ConnectionLoopBackPeer::MaxSendStreamSize() const
    {
        return sendBuffer.max_size();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ConnectionLoopBackPeer::ReceiveStream()
    {
        return streamReader.Emplace(peer);
    }

    void ConnectionLoopBackPeer::AckReceived()
    {
        streamReader->ConsumeRead();

        if (peer.requestedSendSize != 0)
            peer.TryAllocateSendStream();
    }

    void ConnectionLoopBackPeer::CloseAndDestroy()
    {
        ResetOwnership();
        peer.ResetOwnership();
    }

    void ConnectionLoopBackPeer::AbortAndDestroy()
    {
        ResetOwnership();
        peer.ResetOwnership();
    }

    void ConnectionLoopBackPeer::TryAllocateSendStream()
    {
        assert(streamWriter.Allocatable());
        if (sendBuffer.max_size() - sendBuffer.size() >= requestedSendSize)
        {
            auto size = requestedSendSize;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([this, size](const infra::SharedPtr<ConnectionLoopBack>& loopBack)
            {
                if (HasObserver())
                {
                    infra::SharedPtr<infra::StreamWriter> writer = streamWriter.Emplace(*this, size);
                    GetObserver().SendStreamAvailable(std::move(writer));
                }
            }, loopBack.SharedFromThis());

            requestedSendSize = 0;
        }
    }

    ConnectionLoopBackPeer::StreamWriterLoopBack::StreamWriterLoopBack(ConnectionLoopBackPeer& connection, std::size_t size)
        : infra::LimitedStreamWriter(vectorStreamWriter, size)
        , connection(connection)
    {}

    ConnectionLoopBackPeer::StreamWriterLoopBack::~StreamWriterLoopBack()
    {
        if (!vectorStreamWriter.Storage().empty())
        {
            connection.sendBuffer.insert(connection.sendBuffer.end(), vectorStreamWriter.Storage().begin(), vectorStreamWriter.Storage().end());

            ConnectionLoopBackPeer& connection = this->connection;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([&connection](const infra::SharedPtr<ConnectionLoopBack>& loopBack)
            {
                if (connection.peer.HasObserver())
                    connection.peer.GetObserver().DataReceived();
            }, connection.loopBack.SharedFromThis());
        }
    }

    ConnectionLoopBackPeer::StreamReaderLoopBack::StreamReaderLoopBack(ConnectionLoopBackPeer& connection)
        : infra::BoundedDequeInputStreamReader(connection.sendBuffer)
        , connection(connection)
    {}

    void ConnectionLoopBackPeer::StreamReaderLoopBack::ConsumeRead()
    {
        connection.sendBuffer.erase(connection.sendBuffer.begin(), connection.sendBuffer.begin() + ConstructSaveMarker());
        Rewind(0);
    }

    ConnectionLoopBack::ConnectionLoopBack()
        : server(client, *this)
        , client(server, *this)
    {}

    Connection& ConnectionLoopBack::Server()
    {
        return server;
    }

    Connection& ConnectionLoopBack::Client()
    {
        return client;
    }

    void ConnectionLoopBack::Connect(infra::SharedPtr<services::ConnectionObserver> serverObserver, infra::SharedPtr<services::ConnectionObserver> clientObserver)
    {
        serverObserver->Attach(Server());
        Server().SetOwnership(SharedFromThis(), serverObserver);
        clientObserver->Attach(Client());
        Client().SetOwnership(SharedFromThis(), clientObserver);
        Client().GetObserver().Connected();
        Server().GetObserver().Connected();
    }

    ConnectionLoopBackListener::ConnectionLoopBackListener(uint16_t port, ConnectionLoopBackFactory& loopBackFactory, ServerConnectionObserverFactory& connectionObserverFactory)
        : port(port)
        , loopBackFactory(loopBackFactory)
        , connectionObserverFactory(connectionObserverFactory)
    {
        loopBackFactory.RegisterListener(port, this);
    }

    ConnectionLoopBackListener::~ConnectionLoopBackListener()
    {
        loopBackFactory.UnregisterListener(port);
    }

    ConnectionLoopBackConnector::ConnectionLoopBackConnector(ConnectionLoopBackFactory& loopBackFactory, ClientConnectionObserverFactory& connectionObserverFactory)
        : loopBackFactory(loopBackFactory)
        , connectionObserverFactory(connectionObserverFactory)
    {}

    void ConnectionLoopBackConnector::Connect()
    {
        infra::EventDispatcherWithWeakPtr::Instance().Schedule([](const infra::SharedPtr<ConnectionLoopBackConnector>& object)
        {
            auto listener = object->loopBackFactory.listeners.find(object->connectionObserverFactory.Port());
            if (listener != object->loopBackFactory.listeners.end())
                listener->second->Accept(object->connectionObserverFactory);
            else
                object->connectionObserverFactory.ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::refused);
        }, SharedFromThis());
    }

    void ConnectionLoopBackListener::Accept(ClientConnectionObserverFactory& clientObserverFactory)
    {
        struct Info
        {
            infra::SharedPtr<ConnectionLoopBack> connection;
            ClientConnectionObserverFactory& clientObserverFactory;
            infra::SharedPtr<services::ConnectionObserver> serverObserver;
        };

        Info info{ infra::MakeSharedOnHeap<ConnectionLoopBack>(), clientObserverFactory };

        connectionObserverFactory.ConnectionAccepted([&info](infra::SharedPtr<services::ConnectionObserver> serverObserver)
        {
            if (serverObserver)
            {
                info.serverObserver = serverObserver;

                info.clientObserverFactory.ConnectionEstablished([&info](infra::SharedPtr<services::ConnectionObserver> clientObserver)
                {
                    if (clientObserver)
                        info.connection->Connect(info.serverObserver, clientObserver);
                });
            }
            else
                info.clientObserverFactory.ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::connectionAllocationFailed);
        }, services::IPv4AddressLocalHost());
    }

    ConnectionLoopBackFactory::~ConnectionLoopBackFactory()
    {
        assert(listeners.empty());
    }

    void ConnectionLoopBackFactory::RegisterListener(uint16_t port, ConnectionLoopBackListener* listener)
    {
        assert(listeners.find(port) == listeners.end());
        listeners.insert(std::make_pair(port, listener));
    }

    void ConnectionLoopBackFactory::UnregisterListener(uint16_t port)
    {
        assert(listeners.find(port) != listeners.end());
        listeners.erase(port);
    }

    infra::SharedPtr<void> ConnectionLoopBackFactory::Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
    {
        return infra::MakeSharedOnHeap<ConnectionLoopBackListener>(port, *this, factory);
    }

    void ConnectionLoopBackFactory::Connect(ClientConnectionObserverFactory& factory)
    {
        infra::SharedPtr<ConnectionLoopBackConnector> connector = infra::MakeSharedOnHeap<ConnectionLoopBackConnector>(*this, factory);
        connector->Connect();
        connectors.push_back(connector);
    }

    void ConnectionLoopBackFactory::CancelConnect(ClientConnectionObserverFactory& factory)
    {
        std::abort();
    }
}
