#ifndef SERVICES_SINGLE_CONNECTION_LISTENER_HPP
#define SERVICES_SINGLE_CONNECTION_LISTENER_HPP

#include "infra/util/ProxyCreator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include "services/util/Stoppable.hpp"

namespace services
{
    class SingleConnectionListener
        : public Stoppable
        , private ServerConnectionObserverFactory
    {
    public:
        struct Creators
        {
            infra::CreatorBase<services::ConnectionObserver, void(IPAddress address)>& connectionCreator;
        };

        SingleConnectionListener(ConnectionFactory& connectionFactory, uint16_t port, const Creators& creators);

        // Implementation of Stoppable
        virtual void Stop(const infra::Function<void()>& onDone) override;

    private:
        // Implementation of ServerConnectionObserverFactory
        virtual void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address) override;

        void Stop(const infra::Function<void()>& onDone, bool force);
        void CreateObserver();

    private:
        decltype(Creators::connectionCreator) connectionCreator;
        infra::NotifyingSharedOptional<infra::ProxyCreator<decltype(Creators::connectionCreator)>> connection;
        infra::SharedPtr<void> listener;

        infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver;
        IPAddress address;
    };
}

#endif
