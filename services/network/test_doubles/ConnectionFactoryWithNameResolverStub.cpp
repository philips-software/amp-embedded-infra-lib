#include "services/network/test_doubles/ConnectionFactoryWithNameResolverStub.hpp"

namespace services
{
    ConnectionFactoryWithNameResolverStub::ConnectionFactoryWithNameResolverStub(services::ConnectionFactory& connectionFactory, services::IPAddress address)
        : connectionFactory(connectionFactory)
        , address(address)
    {}

    void ConnectionFactoryWithNameResolverStub::Connect(services::ClientConnectionObserverFactoryWithNameResolver& factory)
    {
        client = &factory;
        connectionFactory.Connect(*this);
    }

    void ConnectionFactoryWithNameResolverStub::CancelConnect(services::ClientConnectionObserverFactoryWithNameResolver& factory)
    {
        connectionFactory.CancelConnect(*this);
    }

    services::IPAddress ConnectionFactoryWithNameResolverStub::Address() const
    {
        return address;
    }

    uint16_t ConnectionFactoryWithNameResolverStub::Port() const
    {
        return client->Port();
    }

    void ConnectionFactoryWithNameResolverStub::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        client->ConnectionEstablished(std::move(createdObserver));
    }

    void ConnectionFactoryWithNameResolverStub::ConnectionFailed(ConnectFailReason reason)
    {
        client->ConnectionFailed(reason == ConnectFailReason::connectionAllocationFailed ? services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::connectionAllocationFailed
                                                                                         : services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::refused);
    }
}
