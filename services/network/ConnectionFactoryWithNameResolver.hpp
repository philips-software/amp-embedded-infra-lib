#ifndef SERVICES_CONNECTION_FACTORY_WITH_NAME_RESOLVER_HPP
#define SERVICES_CONNECTION_FACTORY_WITH_NAME_RESOLVER_HPP

#include "infra/util/BoundedList.hpp"
#include "services/network/Connection.hpp"
#include "services/network/NameResolver.hpp"

namespace services
{
    class ClientConnectionObserverFactoryWithNameResolver
        : public infra::IntrusiveList<ClientConnectionObserverFactoryWithNameResolver>::NodeType
    {
    protected:
        ClientConnectionObserverFactoryWithNameResolver() = default;
        ClientConnectionObserverFactoryWithNameResolver(const ClientConnectionObserverFactoryWithNameResolver& other) = delete;
        ClientConnectionObserverFactoryWithNameResolver& operator=(const ClientConnectionObserverFactoryWithNameResolver& other) = delete;
        ~ClientConnectionObserverFactoryWithNameResolver() = default;

    public:
        enum ConnectFailReason
        {
            refused,
            connectionAllocationFailed,
            nameLookupFailed
        };

        virtual infra::BoundedConstString Hostname() const = 0;
        virtual uint16_t Port() const = 0;

        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) = 0;
        virtual void ConnectionFailed(ConnectFailReason reason) = 0;

        static const char* ToString(ConnectFailReason reason);
    };

    class ConnectionFactoryWithNameResolver
    {
    protected:
        ConnectionFactoryWithNameResolver() = default;
        ConnectionFactoryWithNameResolver(const ConnectionFactoryWithNameResolver& other) = delete;
        ConnectionFactoryWithNameResolver& operator=(const ConnectionFactoryWithNameResolver& other) = delete;
        ~ConnectionFactoryWithNameResolver() = default;

    public:
        virtual void Connect(ClientConnectionObserverFactoryWithNameResolver& factory) = 0;
        virtual void CancelConnect(ClientConnectionObserverFactoryWithNameResolver& factory) = 0;
    };

    class ConnectionFactoryWithNameResolverImpl
        : public ConnectionFactoryWithNameResolver
    {
    protected:
        class Action
            : public NameResolverResult
            , public ClientConnectionObserverFactory
        {
        public:
            Action(ConnectionFactoryWithNameResolverImpl& connectionFactory, ClientConnectionObserverFactoryWithNameResolver& clientConnectionFactory);

            bool Remove(ClientConnectionObserverFactoryWithNameResolver& clientConnectionFactory);

            // Implementation of NameResolverResult
            virtual infra::BoundedConstString Hostname() const override;
            virtual void NameLookupDone(IPAddress address, infra::TimePoint validUntil) override;
            virtual void NameLookupFailed() override;

            // Implementation of ClientConnectionObserverFactory
            virtual IPAddress Address() const override;
            virtual uint16_t Port() const override;
            virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
            virtual void ConnectionFailed(ConnectFailReason reason) override;

        private:
            ConnectionFactoryWithNameResolverImpl& connectionFactory;
            ClientConnectionObserverFactoryWithNameResolver& clientConnectionFactory;
            IPAddress address;
            bool connecting = false;
        };

    public:
        template<std::size_t NumParallelActions>
            using WithStorage = infra::WithStorage<ConnectionFactoryWithNameResolverImpl
                , infra::BoundedList<Action>::WithMaxSize<NumParallelActions>>;

        ConnectionFactoryWithNameResolverImpl(infra::BoundedList<Action>& actions, ConnectionFactory& connectionFactory, NameResolver& nameLookup);

        // Implementation of ConnectionFactoryWithNameResolver
        virtual void Connect(ClientConnectionObserverFactoryWithNameResolver& factory) override;
        virtual void CancelConnect(ClientConnectionObserverFactoryWithNameResolver& factory) override;

    protected:
        virtual void NameLookupFailed();
        virtual void NameLookupSuccessful(IPAddress address);

    private:
        void CheckNameLookup();
        void ActionIsDone(Action& action);

    private:
        ConnectionFactory& connectionFactory;
        NameResolver& nameLookup;

        infra::IntrusiveList<ClientConnectionObserverFactoryWithNameResolver> waitingActions;
        infra::BoundedList<Action>& actions;
    };
}

#endif
