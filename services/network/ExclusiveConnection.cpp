#include "services/network/ExclusiveConnection.hpp"

namespace services
{
    infra::ClaimableResource& ExclusiveConnectionFactoryMutex::Resource()
    {
        return resource;
    }

    infra::SharedPtr<ConnectionObserver> ExclusiveConnectionFactoryMutex::CreateConnection(ExclusiveConnectionFactoryMutex::Claimer&& claimer,
        infra::SharedPtr<ConnectionObserver> connectionObserver, bool cancelConnectionOnNewRequest)
    {
        this->cancelConnectionOnNewRequest = cancelConnectionOnNewRequest;
        really_assert(exclusiveConnection.Allocatable());
        currentClaimer = std::move(claimer);
        auto result = exclusiveConnection.Emplace(*this);

        connectionObserver->Attach(*result);
        result->SetOwnership(nullptr, connectionObserver);

        return result;
    }

    void ExclusiveConnectionFactoryMutex::RequestCloseConnection()
    {
        if (exclusiveConnection && cancelConnectionOnNewRequest)
            exclusiveConnection->Close();
    }

    ExclusiveConnectionFactoryMutex::ExclusiveConnection::ExclusiveConnection(ExclusiveConnectionFactoryMutex& mutex)
        : mutex(mutex)
    {}

    ExclusiveConnectionFactoryMutex::ExclusiveConnection::~ExclusiveConnection()
    {
        ResetOwnership();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::RequestSendStream(std::size_t sendSize)
    {
        return ConnectionObserver::Subject().RequestSendStream(sendSize);
    }

    std::size_t ExclusiveConnectionFactoryMutex::ExclusiveConnection::MaxSendStreamSize() const
    {
        return ConnectionObserver::Subject().MaxSendStreamSize();
    }

    infra::SharedPtr<infra::StreamReaderWithRewinding> ExclusiveConnectionFactoryMutex::ExclusiveConnection::ReceiveStream()
    {
        return ConnectionObserver::Subject().ReceiveStream();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::AckReceived()
    {
        ConnectionObserver::Subject().AckReceived();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::CloseAndDestroy()
    {
        ConnectionObserver::Subject().CloseAndDestroy();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::AbortAndDestroy()
    {
        ConnectionObserver::Subject().AbortAndDestroy();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::SetHostname(infra::BoundedConstString hostname)
    {
        static_cast<ConnectionWithHostname&>(ConnectionObserver::Subject()).SetHostname(hostname);
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& streamWriter)
    {
        Connection::GetObserver().SendStreamAvailable(std::move(streamWriter));
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::DataReceived()
    {
        Connection::GetObserver().DataReceived();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::Connected()
    {
        Connection::GetObserver().Connected();
        if (mutex.resource.ClaimsPending() && mutex.cancelConnectionOnNewRequest)
            Connection::GetObserver().Close();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::ClosingConnection()
    {
        ResetOwnership();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::Close()
    {
        if (!invokedClose)
            Connection::GetObserver().Close();
        invokedClose = true;
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::Abort()
    {
        Connection::GetObserver().Abort();
    }

    ExclusiveConnectionFactory::ExclusiveConnectionFactory(infra::BoundedList<infra::NotifyingSharedOptional<Listener>>& listeners, infra::BoundedList<Connector>& connectors,
        ExclusiveConnectionFactoryMutex& mutex, ConnectionFactory& connectionFactory, bool cancelConnectionOnNewRequest)
        : mutex(mutex)
        , listeners(listeners)
        , connectors(connectors)
        , connectionFactory(connectionFactory)
        , cancelConnectionOnNewRequest(cancelConnectionOnNewRequest)
    {}

    infra::SharedPtr<void> ExclusiveConnectionFactory::Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
    {
        assert(!listeners.full());
        listeners.emplace_back();
        auto& index = listeners.back();
        listeners.back().OnAllocatable([this, &index]() { listeners.remove(index); });
        return listeners.back().Emplace(*this, port, factory, versions);
    }

    void ExclusiveConnectionFactory::Connect(ClientConnectionObserverFactory& factory)
    {
        connectors.emplace_back(*this, factory);
    }

    void ExclusiveConnectionFactory::CancelConnect(ClientConnectionObserverFactory& factory)
    {
        for (auto& connector : connectors)
            if (connector.CancelConnect(factory))
            {
                connectors.remove(connector);
                return;
            }
    }

    ExclusiveConnectionFactory::Listener::Listener(ExclusiveConnectionFactory& connectionFactory, uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions)
        : connectionFactory(connectionFactory)
        , factory(factory)
        , claimer(connectionFactory.mutex.Resource())
        , listener(connectionFactory.connectionFactory.Listen(port, *this, versions))
    {}

    void ExclusiveConnectionFactory::Listener::ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address)
    {
        if (!claimer.IsQueued())
        {
            connectionFactory.mutex.RequestCloseConnection();

            auto created = createdObserver.Clone();
            claimer.Claim([this, address, created]()
            {
                this->createdObserver = created;
                factory.ConnectionAccepted([this](infra::SharedPtr<ConnectionObserver> connectionObserver)
                {
                    this->createdObserver(connectionFactory.mutex.CreateConnection(std::move(claimer), connectionObserver, connectionFactory.cancelConnectionOnNewRequest));
                }, address);
            });
        }
    }

    ExclusiveConnectionFactory::Connector::Connector(ExclusiveConnectionFactory& connectionFactory, ClientConnectionObserverFactory& clientFactory)
        : connectionFactory(connectionFactory)
        , clientFactory(clientFactory)
        , claimer(connectionFactory.mutex.Resource())
    {
        connectionFactory.mutex.RequestCloseConnection();

        claimer.Claim([this]()
        {
            this->connectionFactory.connectionFactory.Connect(*this);
        });
    }

    bool ExclusiveConnectionFactory::Connector::CancelConnect(ClientConnectionObserverFactory& factory)
    {
        if (&factory == &clientFactory)
        {
            if (!claimer.IsQueued() && createdObserver == nullptr)
                connectionFactory.connectionFactory.CancelConnect(*this);

            claimer.Release();
            return true;
        }

        return false;
    }

    IPAddress ExclusiveConnectionFactory::Connector::Address() const
    {
        return clientFactory.Address();
    }

    uint16_t ExclusiveConnectionFactory::Connector::Port() const
    {
        return clientFactory.Port();
    }

    void ExclusiveConnectionFactory::Connector::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        this->createdObserver = std::move(createdObserver);
        clientFactory.ConnectionEstablished([this](infra::SharedPtr<ConnectionObserver> connectionObserver)
        {
            this->createdObserver(connectionFactory.mutex.CreateConnection(std::move(claimer), connectionObserver, connectionFactory.cancelConnectionOnNewRequest));

            connectionFactory.connectors.remove(*this);
        });
    }

    void ExclusiveConnectionFactory::Connector::ConnectionFailed(ConnectFailReason reason)
    {
        auto& clientFactory = this->clientFactory;
        connectionFactory.connectors.remove(*this);
        clientFactory.ConnectionFailed(reason);
    }
}
