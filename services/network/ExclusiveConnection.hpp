#ifndef SERVICES_EXCLUSIVE_CONNECTION_HPP
#define SERVICES_EXCLUSIVE_CONNECTION_HPP

#include "infra/event/ClaimableResource.hpp"
#include "infra/util/BoundedList.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class ExclusiveConnectionFactoryMutex
    {
    private:
        class ExclusiveConnection;

    public:
        using Claimer = infra::ClaimableResource::Claimer::WithSize<sizeof(void*) + sizeof(IPAddress) + sizeof(infra::AutoResetFunction<void()>)>;

        infra::ClaimableResource& Resource();
        infra::SharedPtr<ExclusiveConnection> CreateConnection(Claimer&& claimer, bool cancelConnectionOnNewRequest);
        void RequestCloseConnection();

    private:
        class ExclusiveConnection
            : public ConnectionWithHostname
            , public ConnectionObserver
        {
        public:
            ExclusiveConnection(ExclusiveConnectionFactoryMutex& mutex);

            // Implementation of Connection
            virtual void RequestSendStream(std::size_t sendSize) override;
            virtual std::size_t MaxSendStreamSize() const override;
            virtual infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
            virtual void AckReceived() override;
            virtual void CloseAndDestroy() override;
            virtual void AbortAndDestroy() override;
            virtual void SetHostname(infra::BoundedConstString hostname) override;

            // Implementation of ConnectionObserver
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& streamWriter) override;
            virtual void DataReceived() override;
            virtual void Detaching() override;
            virtual void Close() override;
            virtual void Abort() override;

            void Attach(const infra::SharedPtr<ConnectionObserver>& observer);

        private:
            ExclusiveConnectionFactoryMutex& mutex;
            bool invokedClose = false;
            bool closing = false;
            bool aborting = false;
        };

        infra::ClaimableResource resource;
        Claimer currentClaimer{ resource };
        infra::NotifyingSharedOptional<ExclusiveConnection> exclusiveConnection{ [this]() { currentClaimer.Release(); } };
        bool cancelConnectionOnNewRequest;
    };

    class ExclusiveConnectionFactory
        : public ConnectionFactory
    {
    private:
        class Listener;
        class Connector;

    public:
        template<std::size_t NumListeners, std::size_t NumConnectors>
        using WithListenersAndConnectors = infra::WithStorage<infra::WithStorage<ExclusiveConnectionFactory,
                                                                  infra::BoundedList<infra::NotifyingSharedOptional<Listener>>::WithMaxSize<NumListeners>>,
            infra::BoundedList<Connector>::WithMaxSize<NumConnectors>>;

        ExclusiveConnectionFactory(infra::BoundedList<infra::NotifyingSharedOptional<Listener>>& listeners, infra::BoundedList<Connector>& connectors,
            ExclusiveConnectionFactoryMutex& mutex, ConnectionFactory& connectionFactory, bool cancelConnectionOnNewRequest);

        // Implementation of ConnectionFactory
        virtual infra::SharedPtr<void> Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions = IPVersions::both) override;
        virtual void Connect(ClientConnectionObserverFactory& factory) override;
        virtual void CancelConnect(ClientConnectionObserverFactory& factory) override;

    private:
        class Listener
            : public ServerConnectionObserverFactory
        {
        public:
            Listener(ExclusiveConnectionFactory& connectionFactory, uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions);

            // Implementation of ServerConnectionObserverFactory
            virtual void ConnectionAccepted(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver, IPAddress address) override;

        private:
            ExclusiveConnectionFactory& connectionFactory;
            ServerConnectionObserverFactory& factory;
            ExclusiveConnectionFactoryMutex::Claimer claimer;
            infra::SharedPtr<void> listener;
            infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)> createdObserver;
            infra::AccessedBySharedPtr access;
        };

        class Connector
            : public ClientConnectionObserverFactory
        {
        public:
            Connector(ExclusiveConnectionFactory& connectionFactory, ClientConnectionObserverFactory& clientFactory);

            bool CancelConnect(ClientConnectionObserverFactory& factory);

            // Implementation of ClientConnectionObserverFactory
            virtual IPAddress Address() const override;
            virtual uint16_t Port() const override;
            virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
            virtual void ConnectionFailed(ConnectFailReason reason) override;

        private:
            ExclusiveConnectionFactory& connectionFactory;
            ClientConnectionObserverFactory& clientFactory;
            ExclusiveConnectionFactoryMutex::Claimer claimer;
            infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)> createdObserver;
            infra::AccessedBySharedPtr access;
        };

    private:
        ExclusiveConnectionFactoryMutex& mutex;
        infra::BoundedList<infra::NotifyingSharedOptional<Listener>>& listeners;
        infra::BoundedList<Connector>& connectors;
        ConnectionFactory& connectionFactory;
        bool cancelConnectionOnNewRequest;
    };
}

#endif
