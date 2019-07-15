#ifndef SERVICES_SINGLE_CONNECTION_LISTENER_HPP
#define SERVICES_SINGLE_CONNECTION_LISTENER_HPP

#include "infra/util/ProxyCreator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class SingleConnectionListener
        : private ServerConnectionObserverFactory
    {
    public:
        struct Creators
        {
            infra::CreatorBase<services::ConnectionObserver, void()>& connectionCreator;
        };

        SingleConnectionListener(ConnectionFactory& connectionFactory, uint16_t port, const Creators& creators);

    private:
        // Implementation of ServerConnectionObserverFactory
        virtual void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address) override;

        void CreateObserver();

    private:
        decltype(Creators::connectionCreator) connectionCreator;
        infra::NotifyingSharedOptional<infra::ProxyCreator<decltype(Creators::connectionCreator)>> connection;
        infra::SharedPtr<void> listener;

        infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver;
    };
}

#endif
