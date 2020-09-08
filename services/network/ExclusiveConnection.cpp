#include "services/network/ExclusiveConnection.hpp"

namespace services
{
    infra::ClaimableResource& ExclusiveConnectionFactoryMutex::Resource()
    {
        return resource;
    }

    infra::SharedPtr<ExclusiveConnectionFactoryMutex::ExclusiveConnection> ExclusiveConnectionFactoryMutex::CreateConnection(ExclusiveConnectionFactoryMutex::Claimer&& claimer,
        bool cancelConnectionOnNewRequest)
    {
        this->cancelConnectionOnNewRequest = cancelConnectionOnNewRequest;
        really_assert(exclusiveConnection.Allocatable());
        currentClaimer = std::move(claimer);
        return exclusiveConnection.Emplace(*this);
    }

    void ExclusiveConnectionFactoryMutex::RequestCloseConnection()
    {
        if (exclusiveConnection && cancelConnectionOnNewRequest)
            exclusiveConnection->Close();
    }

    ExclusiveConnectionFactoryMutex::ExclusiveConnection::ExclusiveConnection(ExclusiveConnectionFactoryMutex& mutex)
        : mutex(mutex)
    {}

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
        Connection::Observer().SendStreamAvailable(std::move(streamWriter));
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::DataReceived()
    {
        Connection::Observer().DataReceived();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::Detaching()
    {
        ConnectionWithHostname::Detach();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::Close()
    {
        if (!invokedClose)
            Connection::Observer().Close();
        invokedClose = true;
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::Abort()
    {
        Connection::Observer().Abort();
    }

    void ExclusiveConnectionFactoryMutex::ExclusiveConnection::Attach(const infra::SharedPtr<ConnectionObserver>& observer)
    {
        ConnectionWithHostname::Attach(observer);

        if (mutex.resource.ClaimsPending() && mutex.cancelConnectionOnNewRequest)
            Connection::Observer().Close();
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
        , access([this]()
            {
                if (createdObserver != nullptr)
                {
                    createdObserver = nullptr;
                    claimer.Release();
                }
            })
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
                auto self = access.MakeShared(*this);
                factory.ConnectionAccepted([self](infra::SharedPtr<ConnectionObserver> connectionObserver)
                {
                    auto newConnection = self->connectionFactory.mutex.CreateConnection(std::move(self->claimer), self->connectionFactory.cancelConnectionOnNewRequest);
                    self->createdObserver(newConnection);
                    if (newConnection->ConnectionObserver::IsAttached())
                        newConnection->Attach(connectionObserver);
                }, address);
            });
        }
    }

    ExclusiveConnectionFactory::Connector::Connector(ExclusiveConnectionFactory& connectionFactory, ClientConnectionObserverFactory& clientFactory)
        : connectionFactory(connectionFactory)
        , clientFactory(clientFactory)
        , claimer(connectionFactory.mutex.Resource())
        , access([this]()
            {
                if (createdObserver != nullptr)
                {
                    createdObserver = nullptr;
                    claimer.Release();
                }

                this->connectionFactory.connectors.remove(*this);
            })
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
        auto self = access.MakeShared(*this);
        clientFactory.ConnectionEstablished([self](infra::SharedPtr<ConnectionObserver> connectionObserver)
        {
            auto newConnection = self->connectionFactory.mutex.CreateConnection(std::move(self->claimer), self->connectionFactory.cancelConnectionOnNewRequest);
            self->createdObserver(newConnection);
            if (newConnection->ConnectionObserver::IsAttached())
                newConnection->Attach(connectionObserver);
        });
    }

    void ExclusiveConnectionFactory::Connector::ConnectionFailed(ConnectFailReason reason)
    {
        claimer.Release();
        auto& clientFactory = this->clientFactory;
        connectionFactory.connectors.remove(*this);
        clientFactory.ConnectionFailed(reason);
    }
}
