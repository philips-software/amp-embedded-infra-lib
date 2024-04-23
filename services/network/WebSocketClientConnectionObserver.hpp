#ifndef SERVICES_WEB_SOCKET_CLIENT_CONNECTION_OBSERVER_HPP
#define SERVICES_WEB_SOCKET_CLIENT_CONNECTION_OBSERVER_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "infra/util/Variant.hpp"
#include "services/network/Connection.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/WebSocket.hpp"
#include "services/util/Stoppable.hpp"

namespace services
{
    class WebSocketClientObserverFactory
        : public infra::IntrusiveList<WebSocketClientObserverFactory>::NodeType
    {
    protected:
        WebSocketClientObserverFactory() = default;
        WebSocketClientObserverFactory(const WebSocketClientObserverFactory& other) = delete;
        WebSocketClientObserverFactory& operator=(const WebSocketClientObserverFactory& other) = delete;
        ~WebSocketClientObserverFactory() = default;

    public:
        enum ConnectFailReason
        {
            refused,
            connectionAllocationFailed,
            nameLookupFailed,
            upgradeFailed
        };

        virtual infra::BoundedString Url() const = 0;
        virtual uint16_t Port() const = 0;

        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> client)>&& createdClientObserver) = 0;
        virtual void ConnectionFailed(ConnectFailReason reason) = 0;
    };

    class WebSocketClientConnector
    {
    protected:
        WebSocketClientConnector() = default;
        WebSocketClientConnector(const WebSocketClientConnector& other) = delete;
        WebSocketClientConnector& operator=(const WebSocketClientConnector& other) = delete;
        ~WebSocketClientConnector() = default;

    public:
        virtual void Connect(WebSocketClientObserverFactory& factory) = 0;
        virtual void CancelConnect(WebSocketClientObserverFactory& factory, const infra::Function<void()>& onDone) = 0;
    };

    class WebSocketClientConnectionObserver
        : public services::ConnectionObserver
        , public services::Connection
    {
    public:
        explicit WebSocketClientConnectionObserver(infra::BoundedConstString path);

        // Implementation of ConnectionObserver
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;
        void Detaching() override;

        // Implementation of Connection
        void RequestSendStream(std::size_t sendSize) override;
        std::size_t MaxSendStreamSize() const override;
        infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        void AckReceived() override;
        void CloseAndDestroy() override;
        void AbortAndDestroy() override;

    private:
        void StreamWriterAllocatable();
        void TryAllocateSendStream();
        void DiscoverData();
        void DiscoverData(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader);
        void SkipPayloadInDiscovery(infra::DataInputStream& stream);
        void SearchForPingRequests();
        void SendPong(infra::SharedPtr<infra::StreamWriter>&& writer);
        static std::pair<uint8_t, uint32_t> ReadOpcodeAndPayloadLength(infra::DataInputStream& stream);
        static void Skip(infra::StreamReader& reader, std::size_t size);

    private:
        class FrameWriter
            : public infra::LimitedStreamWriter
        {
        public:
            FrameWriter(infra::SharedPtr<infra::StreamWriter>&& writer, std::size_t sendSize);
            FrameWriter(const FrameWriter& other) = delete;
            FrameWriter& operator=(const FrameWriter& other) = delete;
            ~FrameWriter();

        private:
            infra::SharedPtr<infra::StreamWriter> writer;
            std::size_t positionAtStart;
        };

        class FrameReader
            : public infra::StreamReaderWithRewinding
        {
        public:
            explicit FrameReader(WebSocketClientConnectionObserver& client);

            void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
            infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
            infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
            bool Empty() const override;
            std::size_t Available() const override;
            std::size_t ConstructSaveMarker() const override;
            void Rewind(std::size_t marker) override;

            void AckReceived();
            infra::SharedPtr<infra::StreamReaderWithRewinding> Reader();

        private:
            void ReadChunk(infra::ByteRange& range);
            void DiscoverNextFrame();
            void Forward(std::size_t amount);

        private:
            WebSocketClientConnectionObserver& client;
            infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
            std::size_t offset = 0;
            std::size_t availableInCurrentFrame;
        };

    private:
        infra::Optional<std::size_t> requestedSendSize;
        infra::NotifyingSharedOptional<FrameWriter> streamWriter;
        infra::SharedOptional<FrameReader> streamReader;
        std::size_t unackedReadAvailable = 0;
        std::size_t availableInCurrentFrame = 0;
        std::size_t saveAtEndOfDiscovery = 0;
        std::size_t skipDiscoveryPayload = 0;
        infra::BoundedVector<uint8_t>::WithMaxSize<8> pongBuffer;
        bool pongRequested = false;
        bool pongStreamRequested = false;
        bool observerStreamRequested = false;
    };

    class HttpClientWebSocketInitiationResult
    {
    protected:
        HttpClientWebSocketInitiationResult() = default;
        HttpClientWebSocketInitiationResult(const HttpClientWebSocketInitiationResult& other) = delete;
        HttpClientWebSocketInitiationResult& operator=(const HttpClientWebSocketInitiationResult& other) = delete;
        ~HttpClientWebSocketInitiationResult() = default;

    public:
        virtual void WebSocketInitiationDone(Connection& connection) = 0;
        virtual void WebSocketInitiationError(WebSocketClientObserverFactory::ConnectFailReason reason) = 0;
    };

    class HttpClientWebSocketInitiation
        : public Stoppable
        , public HttpClientBasic
    {
    public:
        HttpClientWebSocketInitiation(WebSocketClientObserverFactory& clientObserverFactory, HttpClientConnector& clientConnector,
            HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator);
        HttpClientWebSocketInitiation(WebSocketClientObserverFactory& clientObserverFactory, HttpClientConnector& clientConnector,
            HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator, NoAutoConnect);

        using HttpClientBasic::Detach;

        // Implementation of Stoppable
        void Stop(const infra::Function<void()>& onDone) override;

        // Implementation of HttpClientBasic
        void Attached() override;
        services::HttpHeaders Headers() const override;
        void StatusAvailable(HttpStatusCode statusCode) override;
        void HeaderAvailable(HttpHeader header) override;
        void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override;
        void BodyComplete() override;
        void Done() override;
        void Error(bool intermittentFailure) override;
        void CloseConnection() override;
        void ConnectionFailed(HttpClientObserverFactory::ConnectFailReason reason) override;

    private:
        WebSocketClientObserverFactory::ConnectFailReason Translate(HttpClientObserverFactory::ConnectFailReason reason) const;

    private:
        HttpClientWebSocketInitiationResult& result;
        infra::BoundedVector<const services::HttpHeader>::WithMaxSize<6> headers;
        infra::BoundedString::WithStorage<32> webSocketKey;
        WebSocketClientObserverFactory::ConnectFailReason initiationError = WebSocketClientObserverFactory::ConnectFailReason::upgradeFailed;
        bool done = false;
    };

    class WebSocketClientInitiationResult
    {
    protected:
        WebSocketClientInitiationResult() = default;
        WebSocketClientInitiationResult(const WebSocketClientInitiationResult& other) = delete;
        WebSocketClientInitiationResult& operator=(const WebSocketClientInitiationResult& other) = delete;
        ~WebSocketClientInitiationResult() = default;

    public:
        virtual void InitiationDone(services::Connection& connection) = 0;
        virtual void InitiationError(WebSocketClientObserverFactory::ConnectFailReason reason) = 0;
        virtual void InitiationCancelled() = 0;
    };

    class WebSocketClientFactorySingleConnection
        : public WebSocketClientConnector
        , private WebSocketClientInitiationResult
    {
    public:
        struct Creators
        {
            infra::CreatorBase<Stoppable, void(WebSocketClientObserverFactory& clientObserverFactory,
                                              HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)>& httpClientInitiationCreator;
        };

        WebSocketClientFactorySingleConnection(hal::SynchronousRandomDataGenerator& randomDataGenerator, const Creators& creators);

        void Stop(const infra::Function<void()>& onDone);

        // Implementation of WebSocketClientConnector
        void Connect(WebSocketClientObserverFactory& factory) override;
        void CancelConnect(WebSocketClientObserverFactory& factory, const infra::Function<void()>& onDone) override;

    private:
        // Implementation of WebSocketClientInitiationResult
        void InitiationDone(services::Connection& connection) override;
        void InitiationError(WebSocketClientObserverFactory::ConnectFailReason reason) override;
        void InitiationCancelled() override;

    private:
        class WebSocketClientInitiation
            : private HttpClientWebSocketInitiationResult
        {
        public:
            WebSocketClientInitiation(WebSocketClientObserverFactory& clientObserverFactory,
                WebSocketClientInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator, const Creators& creators);

            void CancelConnect(const infra::Function<void()>& onDone);
            WebSocketClientObserverFactory& Factory();

        private:
            // Implementation of HttpClientWebSocketInitiationResult
            void WebSocketInitiationDone(Connection& connection) override;
            void WebSocketInitiationError(WebSocketClientObserverFactory::ConnectFailReason reason) override;

        private:
            WebSocketClientObserverFactory& clientObserverFactory;
            WebSocketClientInitiationResult& result;

        private:
            infra::ProxyCreator<decltype(Creators::httpClientInitiationCreator)> initiationClient;
            infra::Function<void()> onStopped;
        };

    private:
        infra::Optional<WebSocketClientInitiation> initiation;
        infra::NotifyingSharedOptional<WebSocketClientConnectionObserver> webSocket;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        Creators creators;
    };
}

#endif
