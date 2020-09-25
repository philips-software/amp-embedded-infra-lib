#ifndef SERVICES_EXCLUSIVE_STARTING_CONNECTION_HPP
#define SERVICES_EXCLUSIVE_STARTING_CONNECTION_HPP

#include "infra/util/BoundedList.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class ExclusiveStartingConnectionFactoryMutex
    {
    public:
        class ExclusiveStartingConnection
            : public ConnectionWithHostnameDecorator
        {
        public:
            ExclusiveStartingConnection(ExclusiveStartingConnectionFactoryMutex& mutex);
            ~ExclusiveStartingConnection();

            void ResetStarting();

        private:
            ExclusiveStartingConnectionFactoryMutex& mutex;
        };

        class ExclusiveStartingConnectionRelease
            : public ConnectionWithHostnameDecorator
        {
        public:
            ExclusiveStartingConnectionRelease(ExclusiveStartingConnectionFactoryMutex& mutex);
            ~ExclusiveStartingConnectionRelease();

            // Implementation of ConnectionObserver
            virtual void DataReceived() override;

        private:
            ExclusiveStartingConnectionFactoryMutex& mutex;
            bool reportedStarted = false;
        };

        class WaitingConnection
            : public infra::IntrusiveList<WaitingConnection>::NodeType
        {
        public:
            virtual void Create(const infra::SharedPtr<ExclusiveStartingConnection>& connection) = 0;
        };

    public:
        template<std::size_t MaxConnections>
            using WithMaxConnections = infra::WithStorage<ExclusiveStartingConnectionFactoryMutex, infra::SharedObjectAllocatorFixedSize<ExclusiveStartingConnection, void(ExclusiveStartingConnectionFactoryMutex& mutex)>::WithStorage<MaxConnections>>;

        ExclusiveStartingConnectionFactoryMutex(infra::SharedObjectAllocator<ExclusiveStartingConnection, void(ExclusiveStartingConnectionFactoryMutex& mutex)>& connections);

        void QueueConnection(WaitingConnection& waitingConnection);
        void RemoveConnection(WaitingConnection& waitingConnection);
        void Started();

    private:
        void TryAllocateConnection();

    private:
        infra::SharedObjectAllocator<ExclusiveStartingConnection, void(ExclusiveStartingConnectionFactoryMutex& mutex)>& connections;
        infra::IntrusiveList<WaitingConnection> waitingConnections;
        bool starting = false;
        ExclusiveStartingConnection* startingConnection = nullptr;
    };

    class ExclusiveStartingConnectionFactory
        : public ConnectionFactory
    {
    private:
        class Listener;
        class Connector;

    public:
        template<std::size_t NumListeners, std::size_t NumConnectors>
        using WithListenersAndConnectors = infra::WithStorage<infra::WithStorage<ExclusiveStartingConnectionFactory,
            infra::BoundedList<infra::NotifyingSharedOptional<Listener>>::WithMaxSize<NumListeners>>,
            infra::BoundedList<Connector>::WithMaxSize<NumConnectors>>;

        ExclusiveStartingConnectionFactory(infra::BoundedList<infra::NotifyingSharedOptional<Listener>>& listeners, infra::BoundedList<Connector>& connectors,
            ExclusiveStartingConnectionFactoryMutex& mutex, ConnectionFactory& connectionFactory);

        // Implementation of ConnectionFactory
        virtual infra::SharedPtr<void> Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions = IPVersions::both) override;
        virtual void Connect(ClientConnectionObserverFactory& factory) override;
        virtual void CancelConnect(ClientConnectionObserverFactory& factory) override;

    private:
        class Listener
            : public ServerConnectionObserverFactory
            , public ExclusiveStartingConnectionFactoryMutex::WaitingConnection
        {
        public:
            Listener(ExclusiveStartingConnectionFactory& connectionFactory, uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions);
            ~Listener();

            // Implementation of ServerConnectionObserverFactory
            virtual void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address) override;

            // Implementation of ExclusiveStartingConnectionFactoryMutex::WaitingConnection
            virtual void Create(const infra::SharedPtr<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection>& connection) override;

        private:
            ExclusiveStartingConnectionFactory& connectionFactory;
            ServerConnectionObserverFactory& factory;
            infra::SharedPtr<void> listener;
            infra::AccessedBySharedPtr access;
            infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)> createdObserver;
            IPAddress address;
            infra::SharedPtr<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection> connection;
            bool accepting = false;
        };

        class Connector
            : public ClientConnectionObserverFactory
        {
        public:
            Connector(ExclusiveStartingConnectionFactory& connectionFactory, ClientConnectionObserverFactory& clientFactory);

            bool CancelConnect(ClientConnectionObserverFactory& factory);

            // Implementation of ClientConnectionObserverFactory
            virtual IPAddress Address() const override;
            virtual uint16_t Port() const override;
            virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
            virtual void ConnectionFailed(ConnectFailReason reason) override;

        private:
            ExclusiveStartingConnectionFactory& connectionFactory;
            ClientConnectionObserverFactory& clientFactory;
            infra::AccessedBySharedPtr access;
            infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)> createdObserver;
            bool connecting = true;
        };

    private:
        ExclusiveStartingConnectionFactoryMutex& mutex;
        infra::BoundedList<infra::NotifyingSharedOptional<Listener>>& listeners;
        infra::BoundedList<Connector>& connectors;
        ConnectionFactory& connectionFactory;
    };

    class ExclusiveStartingConnectionReleaseFactory
        : public ConnectionFactory
    {
    private:
        class Listener;
        class Connector;

    public:
        template<std::size_t NumListeners, std::size_t NumConnectors, std::size_t NumConnections>
        using WithListenersAndConnectors = infra::WithStorage<infra::WithStorage<infra::WithStorage<ExclusiveStartingConnectionReleaseFactory,
            infra::BoundedList<infra::NotifyingSharedOptional<Listener>>::WithMaxSize<NumListeners>>,
            infra::BoundedList<Connector>::WithMaxSize<NumConnectors>>,
            infra::BoundedList<infra::NotifyingSharedOptional<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnectionRelease>>::WithMaxSize<NumConnections>>;

        ExclusiveStartingConnectionReleaseFactory(infra::BoundedList<infra::NotifyingSharedOptional<Listener>>& listeners, infra::BoundedList<Connector>& connectors,
            infra::BoundedList<infra::NotifyingSharedOptional<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnectionRelease>>& connections,
            ExclusiveStartingConnectionFactoryMutex& mutex, ConnectionFactory& connectionFactory);

        // Implementation of ConnectionFactory
        virtual infra::SharedPtr<void> Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions = IPVersions::both) override;
        virtual void Connect(ClientConnectionObserverFactory& factory) override;
        virtual void CancelConnect(ClientConnectionObserverFactory& factory) override;

    private:
        class Listener
            : public ServerConnectionObserverFactory
        {
        public:
            Listener(ExclusiveStartingConnectionReleaseFactory& connectionFactory, uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions);

            // Implementation of ServerConnectionObserverFactory
            virtual void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address) override;

        private:
            ExclusiveStartingConnectionReleaseFactory& connectionFactory;
            ServerConnectionObserverFactory& factory;
            infra::SharedPtr<void> listener;
            infra::AccessedBySharedPtr access;
            infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)> createdObserver;
        };

        class Connector
            : public ClientConnectionObserverFactory
            , public ExclusiveStartingConnectionFactoryMutex::WaitingConnection
        {
        public:
            Connector(ExclusiveStartingConnectionReleaseFactory& connectionFactory, ClientConnectionObserverFactory& clientFactory);

            bool CancelConnect(ClientConnectionObserverFactory& factory);

            // Implementation of ClientConnectionObserverFactory
            virtual IPAddress Address() const override;
            virtual uint16_t Port() const override;
            virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
            virtual void ConnectionFailed(ConnectFailReason reason) override;

            // Implementation of ExclusiveStartingConnectionFactoryMutex::WaitingConnection
            virtual void Create(const infra::SharedPtr<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection>& connection) override;

        private:
            ExclusiveStartingConnectionReleaseFactory& connectionFactory;
            ClientConnectionObserverFactory& clientFactory;
            infra::AccessedBySharedPtr access;
            infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)> createdObserver;
            infra::SharedPtr<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnection> connection;
        };

    private:
        ExclusiveStartingConnectionFactoryMutex& mutex;
        infra::BoundedList<infra::NotifyingSharedOptional<Listener>>& listeners;
        infra::BoundedList<Connector>& connectors;
        infra::BoundedList<infra::NotifyingSharedOptional<ExclusiveStartingConnectionFactoryMutex::ExclusiveStartingConnectionRelease>>& connections;
        ConnectionFactory& connectionFactory;
    };
}

#endif
