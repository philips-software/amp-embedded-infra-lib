#ifndef NETWORK_CONNECTION_FACTORY_WITH_NAME_RESOLVER_STUB_HPP
#define NETWORK_CONNECTION_FACTORY_WITH_NAME_RESOLVER_STUB_HPP

#include "services/network/ConnectionFactoryWithNameResolver.hpp"

namespace services
{
    class ConnectionFactoryWithNameResolverStub
        : public services::ConnectionFactoryWithNameResolver
        , private services::ClientConnectionObserverFactory
    {
    public:
        ConnectionFactoryWithNameResolverStub(services::ConnectionFactory& connectionFactory, services::IPAddress address);

        // Implementation of ConnectionFactoryWithNameResolver
        void Connect(services::ClientConnectionObserverFactoryWithNameResolver& factory) override;
        void CancelConnect(services::ClientConnectionObserverFactoryWithNameResolver& factory) override;

        // Implementation of ClientConnectionObserverFactory
        services::IPAddress Address() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

    private:
        services::ConnectionFactory& connectionFactory;
        services::IPAddress address;

        services::ClientConnectionObserverFactoryWithNameResolver* client = nullptr;
    };
}

#endif
