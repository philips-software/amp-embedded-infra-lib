#include "services/network/ExclusiveStartingConnection.hpp"

namespace services
{
    ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnectionFactoryMutex(infra::SharedObjectAllocator<ExclusiveStartingConnection, void(ExclusiveStartingConnectionFactoryMutex& mutex)>& connections)
        : connections(connections)
    {}

    void ExclusiveStartingConnectionFactoryMutex::QueueConnection(WaitingConnection& waitingConnection)
    {
        waitingConnections.push_back(waitingConnection);
        TryAllocateConnection();
    }

    void ExclusiveStartingConnectionFactoryMutex::Started()
    {
        starting = false;
        TryAllocateConnection();
    }

    void ExclusiveStartingConnectionFactoryMutex::TryAllocateConnection()
    {
        if (!starting && !waitingConnections.empty())
        {
            auto connection = connections.Allocate(*this);
            if (connection != nullptr)
            {
                starting = true;
                auto& waitingConnection = waitingConnections.front();
                waitingConnections.pop_front();
                waitingConnection.Create(connection);
            }
            else
                connections.OnAllocatable([this]() { TryAllocateConnection(); });
        }
    }

    ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::ExclusiveStartingConnection(ExclusiveStartingConnectionFactoryMutex& mutex)
        : mutex(mutex)
    {}

    ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::~ExclusiveStartingConnection()
    {
        if (!reportedStarted)
            mutex.Started();
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::RequestSendStream(std::size_t sendSize)
    {
        return ConnectionObserver::Subject().RequestSendStream(sendSize);
    }

    std::size_t ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::MaxSendStreamSize() const
    {
        return ConnectionObserver::Subject().MaxSendStreamSize();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::ReceiveStream()
    {
        return ConnectionObserver::Subject().ReceiveStream();
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::AckReceived()
    {
        ConnectionObserver::Subject().AckReceived();
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::CloseAndDestroy()
    {
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::AbortAndDestroy()
    {
        ConnectionObserver::Subject().AbortAndDestroy();
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::SetHostname(infra::BoundedConstString hostname)
    {
        static_cast<ConnectionWithHostname&>(ConnectionObserver::Subject()).SetHostname(hostname);
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& streamWriter)
    {
        Connection::Observer().SendStreamAvailable(std::move(streamWriter));
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::DataReceived()
    {
        if (!reportedStarted)
        {
            reportedStarted = true;
            mutex.Started();
        }

        Connection::Observer().DataReceived();
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::Detaching()
    {
        ConnectionWithHostname::Detach();
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::Close()
    {
        Connection::Observer().Close();
    }

    void ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection::Abort()
    {
        Connection::Observer().Abort();
    }

    ExclusiveStartingConnectionFactory::ExclusiveStartingConnectionFactory(infra::BoundedList<infra::NotifyingSharedOptional<Listener>>& listeners, infra::BoundedList<Connector>& connectors,
        ExclusiveStartingConnectionFactoryMutex& mutex, ConnectionFactory& connectionFactory)
        : mutex(mutex)
        , listeners(listeners)
        , connectors(connectors)
        , connectionFactory(connectionFactory)
    {}

    infra::SharedPtr<void> ExclusiveStartingConnectionFactory::Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
    {
        assert(!listeners.full());
        listeners.emplace_back();
        auto& index = listeners.back();
        listeners.back().OnAllocatable([this, &index]() { listeners.remove(index); });
        return listeners.back().Emplace(*this, port, factory, versions);
    }

    void ExclusiveStartingConnectionFactory::Connect(ClientConnectionObserverFactory& factory)
    {
        connectors.emplace_back(*this, factory);
    }

    void ExclusiveStartingConnectionFactory::CancelConnect(ClientConnectionObserverFactory& factory)
    {
        for (auto& connector : connectors)
            if (connector.CancelConnect(factory))
            {
                connectors.remove(connector);
                return;
            }
    }

    ExclusiveStartingConnectionFactory::Listener::Listener(ExclusiveStartingConnectionFactory& connectionFactory, uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
        : connectionFactory(connectionFactory)
        , factory(factory)
        , listener(connectionFactory.connectionFactory.Listen(port, *this, versions))
    {}

    void ExclusiveStartingConnectionFactory::Listener::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address)
    {
        assert(this->createdObserver == nullptr);

        this->createdObserver = std::move(createdObserver);
        this->address = address;
        connectionFactory.mutex.QueueConnection(*this);
    }

    void ExclusiveStartingConnectionFactory::Listener::Create(const infra::SharedPtr<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection>& connection)
    {
        this->connection = connection;
        factory.ConnectionAccepted([this](infra::SharedPtr<ConnectionObserver> connectionObserver)
        {
            this->createdObserver(this->connection);
            if (this->connection->ConnectionObserver::IsAttached())
                this->connection->Attach(connectionObserver);
            this->connection = nullptr;
        }, address);
    }

    ExclusiveStartingConnectionFactory::Connector::Connector(ExclusiveStartingConnectionFactory& connectionFactory, ClientConnectionObserverFactory& clientFactory)
        : connectionFactory(connectionFactory)
        , clientFactory(clientFactory)
    {
        connectionFactory.connectionFactory.Connect(*this);
    }

    bool ExclusiveStartingConnectionFactory::Connector::CancelConnect(ClientConnectionObserverFactory& factory)
    {
        if (&factory == &clientFactory)
        {
            connectionFactory.connectionFactory.CancelConnect(*this);
            return true;
        }

        return false;
    }

    IPAddress ExclusiveStartingConnectionFactory::Connector::Address() const
    {
        return clientFactory.Address();
    }

    uint16_t ExclusiveStartingConnectionFactory::Connector::Port() const
    {
        return clientFactory.Port();
    }

    void ExclusiveStartingConnectionFactory::Connector::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        this->createdObserver = std::move(createdObserver);
        connectionFactory.mutex.QueueConnection(*this);
    }

    void ExclusiveStartingConnectionFactory::Connector::Create(const infra::SharedPtr<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection>& connection)
    {
        this->connection = connection;

        clientFactory.ConnectionEstablished([this](infra::SharedPtr<ConnectionObserver> connectionObserver)
        {
            this->createdObserver(this->connection);
            if (this->connection->ConnectionObserver::IsAttached())
                this->connection->Attach(connectionObserver);
            this->connection = nullptr;
            connectionFactory.connectors.remove(*this);
        });
    }

    void ExclusiveStartingConnectionFactory::Connector::ConnectionFailed(ConnectFailReason reason)
    {
        auto& clientFactory = this->clientFactory;
        connectionFactory.connectors.remove(*this);
        clientFactory.ConnectionFailed(reason);
    }
}
