#ifndef SERVICES_CONNECTION_WIN_HPP
#define SERVICES_CONNECTION_WIN_HPP

#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/SharedObjectAllocator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/Connection.hpp"
#include <list>
#include <winsock2.h>

namespace services
{
    class EventDispatcherWithNetwork;

    class ConnectionWin
        : public services::Connection
        , public infra::EnableSharedFromThis<ConnectionWin>
    {
    public:
        ConnectionWin(EventDispatcherWithNetwork& network, SOCKET socket);
        ~ConnectionWin();

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
        void UpdateEventFlags();

    private:
        void TryAllocateSendStream();

    private:
        class StreamWriterWin
            : private std::vector<uint8_t>
            , public infra::ByteOutputStreamWriter
        {
        public:
            StreamWriterWin(ConnectionWin& connection, std::size_t size);
            ~StreamWriterWin();

        private:
            ConnectionWin& connection;
        };

        class StreamReaderWin
            : public infra::BoundedDequeInputStreamReader
        {
        public:
            StreamReaderWin(ConnectionWin& connection);

            void ConsumeRead();

        private:
            ConnectionWin& connection;
        };

    private:
        friend class EventDispatcherWithNetwork;

        EventDispatcherWithNetwork& network;
        SOCKET socket;
        WSAEVENT event = WSACreateEvent();

        infra::BoundedDeque<uint8_t>::WithMaxSize<2048> receiveBuffer;
        infra::BoundedDeque<uint8_t>::WithMaxSize<2048> sendBuffer;

        infra::SharedOptional<StreamWriterWin> streamWriter;
        std::size_t requestedSendSize = 0;
        infra::SharedOptional<StreamReaderWin> streamReader;
        bool trySend = false;
    };

    using AllocatorConnectionWin = infra::SharedObjectAllocator<ConnectionWin, void(EventDispatcherWithNetwork&, SOCKET)>;

    class ListenerWin
        : public infra::IntrusiveList<ListenerWin>::NodeType
    {
    public:
        ListenerWin(EventDispatcherWithNetwork& network, uint16_t port, services::ServerConnectionObserverFactory& factory);
        ~ListenerWin();

        void Accept();

    private:
        friend class EventDispatcherWithNetwork;

        EventDispatcherWithNetwork& network;
        services::ServerConnectionObserverFactory& factory;
        SOCKET listenSocket;
        WSAEVENT event = WSACreateEvent();
    };

    class ConnectorWin
        : public infra::IntrusiveList<ConnectorWin>::NodeType
        , public infra::EnableSharedFromThis<ConnectorWin>
    {
    public:
        ConnectorWin(EventDispatcherWithNetwork& network, services::ClientConnectionObserverFactory& factory);
        ~ConnectorWin();

        void Connected();
        void Failed();

    private:
        friend class EventDispatcherWithNetwork;

        EventDispatcherWithNetwork& network;
        services::ClientConnectionObserverFactory& factory;
        SOCKET connectSocket;
        WSAEVENT event = WSACreateEvent();
    };
}

#endif
