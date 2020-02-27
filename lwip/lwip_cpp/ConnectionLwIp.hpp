#ifndef LWIP_CONNECTION_LW_IP_HPP
#define LWIP_CONNECTION_LW_IP_HPP

#include "infra/timer/Timer.hpp"
#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/util/BoundedList.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/WithStorage.hpp"
#include "lwip/tcp.h"
#include "services/network/Address.hpp"
#include "services/network/Connection.hpp"

namespace services
{
#ifdef ESP_PLATFORM
    static const uint32_t tcpSndBuf = 2 * TCP_MSS;
    static const uint32_t tcpSndQueueLen = ((4 * (tcpSndBuf) + (TCP_MSS - 1))/(TCP_MSS));
    static const uint32_t tcpWnd = (4 * TCP_MSS);
#else
    static const uint32_t tcpSndBuf = TCP_SND_BUF;
    static const uint32_t tcpSndQueueLen = TCP_SND_QUEUELEN;
    static const uint32_t tcpWnd = TCP_WND;
#endif

    class ConnectionLwIp
        : public services::Connection
        , public infra::EnableSharedFromThis<ConnectionLwIp>
        , public infra::IntrusiveList<ConnectionLwIp>::NodeType
    {
    public:
        ConnectionLwIp(tcp_pcb* control);
        ~ConnectionLwIp();

        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual std::size_t MaxSendStreamSize() const override;
        virtual infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        virtual void AckReceived() override;
        virtual void CloseAndDestroy() override;
        virtual void AbortAndDestroy() override;

        void SetSelfOwnership(const infra::SharedPtr<ConnectionObserver>& observer);
        void ResetOwnership();
        IPAddress IpAddress() const;

    private:
        void SendBuffer(infra::ConstByteRange buffer);
        void TryAllocateSendStream();
        void ResetControl();

        static err_t Recv(void* arg, tcp_pcb* tpcb, pbuf* p, err_t err);
        static void Err(void* arg, err_t err);
        static err_t Sent(void* arg, struct tcp_pcb* tpcb, uint16_t len);

        err_t Recv(pbuf* p, err_t err);
        void Err(err_t err);
        err_t Sent(uint16_t len);
        void RemoveFromPool(infra::ConstByteRange range);

    private:
        class StreamWriterLwIp
            : public infra::ByteOutputStreamWriter
        {
        public:
            StreamWriterLwIp(ConnectionLwIp& connection, infra::ByteRange sendBuffer);
            ~StreamWriterLwIp();

        private:
            ConnectionLwIp& connection;
        };

        class StreamReaderLwIp
            : public infra::StreamReaderWithRewinding
        {
        public:
            StreamReaderLwIp(ConnectionLwIp& connection);

            void ConsumeRead();

            virtual void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
            virtual infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
            virtual infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
            virtual bool Empty() const override;
            virtual std::size_t Available() const override;
            virtual std::size_t ConstructSaveMarker() const override;
            virtual void Rewind(std::size_t marker) override;

        private:
            ConnectionLwIp& connection;
            std::size_t offset = 0;
            pbuf* currentPbuf;
            std::size_t offsetInCurrentPbuf;
        };

    private:
        friend class ListenerLwIp;
        friend class ConnectorLwIp;

        tcp_pcb* control;
        std::size_t requestedSendSize = 0;

        infra::SharedOptional<StreamWriterLwIp> streamWriter;
        infra::SharedOptional<StreamReaderLwIp> streamReader;

        infra::ConstByteRange sendBuffer;
        infra::TimerSingleShot retrySendTimer;
        infra::BoundedDeque<infra::ConstByteRange>::WithMaxSize<tcpSndQueueLen> sendBuffers;
        static infra::BoundedList<std::array<uint8_t, TCP_MSS>>::WithMaxSize<tcpSndQueueLen> sendMemoryPool;
        static infra::IntrusiveList<ConnectionLwIp> sendMemoryPoolWaiting;

        pbuf* receivedData = nullptr;
        std::size_t consumed = 0;
        bool dataReceivedScheduled = false;

        infra::SharedPtr<void> self;
    };

    using AllocatorConnectionLwIp = infra::SharedObjectAllocator<ConnectionLwIp, void(tcp_pcb*)>;

    class ListenerLwIp
    {
    public:
        template<std::size_t Size>
            using WithFixedAllocator = infra::WithStorage<ListenerLwIp, AllocatorConnectionLwIp::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<Size>>;

        ListenerLwIp(AllocatorConnectionLwIp& allocator, uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions);
        ~ListenerLwIp();

    private:
        static err_t Accept(void* arg, struct tcp_pcb* newPcb, err_t err);

        err_t Accept(tcp_pcb* newPcb, err_t err);

    private:
        AllocatorConnectionLwIp& allocator;
        tcp_pcb* listenPort;
        ServerConnectionObserverFactory& factory;
    };

    using AllocatorListenerLwIp = infra::SharedObjectAllocator<ListenerLwIp, void(AllocatorConnectionLwIp&, uint16_t, ServerConnectionObserverFactory&, IPVersions versions)>;

    class ConnectionFactoryLwIp;

    class ConnectorLwIp
    {
    public:
        ConnectorLwIp(ConnectionFactoryLwIp& factory, ClientConnectionObserverFactory& clientFactory, AllocatorConnectionLwIp& connectionAllocator);
        ~ConnectorLwIp();

    private:
        static err_t StaticConnected(void* arg, tcp_pcb* tpcb, err_t err);
        static void StaticError(void* arg, err_t err);
        err_t Connected();
        void Error(err_t err);

    private:
        friend class ConnectionFactoryLwIp;

        ConnectionFactoryLwIp& factory;
        ClientConnectionObserverFactory& clientFactory;
        AllocatorConnectionLwIp& connectionAllocator;
        tcp_pcb* control;
    };

    class ConnectionFactoryLwIp
        : public ConnectionFactory
    {
    public:
        template<std::size_t MaxListeners, std::size_t MaxConnectors, std::size_t MaxConnections>
            using WithFixedAllocator = infra::WithStorage<infra::WithStorage<infra::WithStorage<ConnectionFactoryLwIp,
                AllocatorListenerLwIp::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxListeners>>,
                infra::BoundedList<ClientConnectionObserverFactory>::WithMaxSize<MaxConnectors>>,
                AllocatorConnectionLwIp::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>;

    public:
        ConnectionFactoryLwIp(AllocatorListenerLwIp& listenerAllocator, infra::BoundedList<ConnectorLwIp>& connectors, AllocatorConnectionLwIp& connectionAllocator);

        virtual infra::SharedPtr<void> Listen(uint16_t port, ServerConnectionObserverFactory& factory, IPVersions versions = IPVersions::both) override;
        virtual void Connect(ClientConnectionObserverFactory& factory) override;
        virtual void CancelConnect(ClientConnectionObserverFactory& factory) override;

        void Remove(ConnectorLwIp& connector);

    private:
        void TryConnect();

    private:
        AllocatorListenerLwIp& listenerAllocator;
        infra::IntrusiveList<ClientConnectionObserverFactory> waitingConnectors;
        infra::BoundedList<ConnectorLwIp>& connectors;
        AllocatorConnectionLwIp& connectionAllocator;
    };
}

#endif
