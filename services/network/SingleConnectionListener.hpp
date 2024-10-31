#ifndef SERVICES_SINGLE_CONNECTION_LISTENER_HPP
#define SERVICES_SINGLE_CONNECTION_LISTENER_HPP

#include "infra/util/ProxyCreator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include "services/util/Stoppable.hpp"

namespace services
{
    class SingleConnectionListener;

    class NewConnectionStrategy
    {
    public:
        NewConnectionStrategy() = default;
        NewConnectionStrategy(const NewConnectionStrategy& other) = delete;
        NewConnectionStrategy& operator=(const NewConnectionStrategy& other) = delete;
        ~NewConnectionStrategy() = default;

        virtual void StopCurrentConnection(SingleConnectionListener& listener) = 0;
        virtual void StartNewConnection() = 0;
    };

    class SingleConnectionListener
        : public Stoppable
        , public NewConnectionStrategy
        , private ServerConnectionObserverFactory
    {
    public:
        struct Creators
        {
            infra::CreatorBase<services::ConnectionObserver, void(IPAddress address)>& connectionCreator;
        };

        SingleConnectionListener(ConnectionFactory& connectionFactory, uint16_t port, const Creators& creators);

        void SetNewConnectionStrategy(NewConnectionStrategy& newConnectionStrategy);

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        // Implementation of NewConnectionStrategy
        void StopCurrentConnection(SingleConnectionListener& listener) override;
        void StartNewConnection() override;

    private:
        // Implementation of ServerConnectionObserverFactory
        void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address) override;

        void Stop(const infra::Function<void()>& onDone, bool force);

    private:
        NewConnectionStrategy* newConnectionStrategy = this;
        decltype(Creators::connectionCreator) connectionCreator;
        infra::NotifyingSharedOptional<infra::ProxyCreator<decltype(Creators::connectionCreator)>> connection;
        infra::SharedPtr<void> listener;

        infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver;
        IPAddress address;
    };
}

#endif
