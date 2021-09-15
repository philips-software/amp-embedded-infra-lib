#ifndef SERVICES_CONNECTION_BSD_HPP
#define SERVICES_CONNECTION_BSD_HPP

#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/SharedObjectAllocator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include <list>

namespace services
{
    class EventDispatcherWithNetwork;

    class ConnectionBsd
        : public services::Connection
        , public infra::EnableSharedFromThis<ConnectionBsd>
    {
    public:
        ConnectionBsd(EventDispatcherWithNetwork& network, int socket);
        ~ConnectionBsd();

        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual std::size_t MaxSendStreamSize() const override;
        virtual infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        virtual void AckReceived() override;
        virtual void CloseAndDestroy() override;
        virtual void AbortAndDestroy() override;

        IPv4Address Ipv4Address() const;
        void SetObserver(infra::SharedPtr<services::ConnectionObserver> connectionObserver);

        void Receive();
        void Send();
        void TrySend();

    private:
        void SetSelfOwnership(const infra::SharedPtr<ConnectionObserver>& observer);
        void ResetOwnership();
        void TryAllocateSendStream();

    private:
        class StreamWriterBsd
            : private std::vector<uint8_t>
            , public infra::ByteOutputStreamWriter
        {
        public:
            StreamWriterBsd(ConnectionBsd& connection, std::size_t size);
            ~StreamWriterBsd();

        private:
            ConnectionBsd& connection;
        };

        class StreamReaderBsd
            : public infra::BoundedDequeInputStreamReader
        {
        public:
            StreamReaderBsd(ConnectionBsd& connection);

            void ConsumeRead();

        private:
            ConnectionBsd& connection;
        };

    private:
        friend class EventDispatcherWithNetwork;

        EventDispatcherWithNetwork& network;
        int socket;

        infra::BoundedDeque<uint8_t>::WithMaxSize<2048> receiveBuffer;
        infra::BoundedDeque<uint8_t>::WithMaxSize<2048> sendBuffer;

        infra::SharedOptional<StreamWriterBsd> streamWriter;
        std::size_t requestedSendSize = 0;
        infra::SharedOptional<StreamReaderBsd> streamReader;
        bool trySend = false;

        infra::SharedPtr<void> self;
    };

    using AllocatorConnectionBsd = infra::SharedObjectAllocator<ConnectionBsd, void(EventDispatcherWithNetwork&, int)>;

    class ListenerBsd
        : public infra::IntrusiveList<ListenerBsd>::NodeType
    {
    public:
        ListenerBsd(EventDispatcherWithNetwork& network, uint16_t port, services::ServerConnectionObserverFactory& factory);
        ListenerBsd(const ListenerBsd& other) = delete;
        ListenerBsd& operator=(const ListenerBsd& other) = delete;
        ~ListenerBsd();

        void Accept();

    private:
        friend class EventDispatcherWithNetwork;

        EventDispatcherWithNetwork& network;
        services::ServerConnectionObserverFactory& factory;
        int listenSocket;
    };

    class ConnectorBsd
        : public infra::IntrusiveList<ConnectorBsd>::NodeType
        , public infra::EnableSharedFromThis<ConnectorBsd>
    {
    public:
        ConnectorBsd(EventDispatcherWithNetwork& network, services::ClientConnectionObserverFactory& factory);
        ConnectorBsd(const ConnectorBsd& other) = delete;
        ConnectorBsd& operator=(const ConnectorBsd& other) = delete;
        ~ConnectorBsd();

        void Connected();
        void Failed();

    private:
        friend class EventDispatcherWithNetwork;

        EventDispatcherWithNetwork& network;
        services::ClientConnectionObserverFactory& factory;
        int connectSocket;
    };

}

#endif
