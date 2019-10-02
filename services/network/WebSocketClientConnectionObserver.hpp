#ifndef SERVICES_WEB_SOCKET_CLIENT_CONNECTION_OBSERVER_HPP
#define SERVICES_WEB_SOCKET_CLIENT_CONNECTION_OBSERVER_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "infra/util/Variant.hpp"
#include "services/network/Connection.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClientBasic.hpp"
#include "services/network/HttpClientImpl.hpp"
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
        virtual void CancelConnect(WebSocketClientObserverFactory& factory) = 0;
    };

    class WebSocketClientConnectionObserver
        : public services::ConnectionObserver
        , public services::Connection
    {
    public:
        WebSocketClientConnectionObserver(infra::BoundedConstString path, services::Connection& connection);
        ~WebSocketClientConnectionObserver();

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void ClosingConnection() override;

        // Implementation of Connection
        virtual void RequestSendStream(std::size_t sendSize) override;
        virtual std::size_t MaxSendStreamSize() const override;
        virtual infra::SharedPtr<infra::StreamReaderWithRewinding> ReceiveStream() override;
        virtual void AckReceived() override;
        virtual void CloseAndDestroy() override;
        virtual void AbortAndDestroy() override;

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
            ~FrameWriter();

        private:
            infra::SharedPtr<infra::StreamWriter> writer;
            std::size_t positionAtStart;
        };

        class FrameReader
            : public infra::StreamReaderWithRewinding
        {
        public:
            FrameReader(WebSocketClientConnectionObserver& client);

            virtual void Extract(infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
            virtual uint8_t Peek(infra::StreamErrorPolicy& errorPolicy) override;
            virtual infra::ConstByteRange ExtractContiguousRange(std::size_t max) override;
            virtual infra::ConstByteRange PeekContiguousRange(std::size_t start) override;
            virtual bool Empty() const override;
            virtual std::size_t Available() const override;
            virtual std::size_t ConstructSaveMarker() const override;
            virtual void Rewind(std::size_t marker) override;

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
        virtual void WebSocketInitiationDone() = 0;
        virtual void WebSocketInitiationError(bool intermittentFailure) = 0;
    };

    class HttpClientWebSocketInitiation
        : public Stoppable
        , public HttpClientBasic
    {
    public:
        HttpClientWebSocketInitiation(WebSocketClientObserverFactory& clientObserverFactory, HttpClientConnector& clientConnector,
            HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        using HttpClientBasic::Detach;

        // Implementation of Stoppable
        virtual void Stop(const infra::Function<void()>& onDone) override;

        // Implementation of HttpClientBasic
        virtual void Connected() override;
        virtual services::HttpHeaders Headers() const;
        virtual void StatusAvailable(HttpStatusCode statusCode) override;
        virtual void HeaderAvailable(HttpHeader header) override;
        virtual void BodyAvailable(infra::SharedPtr<infra::StreamReader>&& reader) override;
        virtual void BodyComplete() override;
        virtual void Done() override;
        virtual void Error(bool intermittentFailure) override;

    private:
        HttpClientWebSocketInitiationResult& result;
        infra::BoundedVector<const services::HttpHeader>::WithMaxSize<4> headers;
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
        virtual void InitiationExpired() = 0;
        virtual void InitiationError() = 0;
        virtual void InitiationConnectionFailed(ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason) = 0;
        virtual void InitiationCancelled() = 0;
    };

    class WebSocketClientInitiation
        : private HttpClientConnector
        , private ClientConnectionObserverFactoryWithNameResolver
        , private HttpClientWebSocketInitiationResult
    {
    public:
        struct Creators
        {
            infra::CreatorBase<HttpClientWebSocketInitiation, void(WebSocketClientObserverFactory& clientObserverFactory, HttpClientConnector& clientConnector,
                HttpClientWebSocketInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator)>& httpClientInitiationCreator;
        };

        WebSocketClientInitiation(WebSocketClientObserverFactory& clientObserverFactory, ConnectionFactoryWithNameResolver& connectionFactory,
            WebSocketClientInitiationResult& result, hal::SynchronousRandomDataGenerator& randomDataGenerator, const Creators& creators);

        void CancelConnect();
        WebSocketClientObserverFactory& Factory();

    private:
        // Implementation of HttpClientConnector
        virtual void Connect(HttpClientObserverFactory& factory) override;
        virtual void CancelConnect(HttpClientObserverFactory& factory) override;

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        virtual infra::BoundedConstString Hostname() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

        // Implementation of HttpClientWebSocketInitiationResult
        virtual void WebSocketInitiationDone() override;
        virtual void WebSocketInitiationError(bool intermittentFailure) override;

        void ExpiredWithSuccess();
        void ExpiredWithError();
        void ExpiredWithConnectionFailed(ConnectFailReason reason);
        void ExpiredWithCancelled();

    private:
        WebSocketClientObserverFactory& clientObserverFactory;
        ConnectionFactoryWithNameResolver& connectionFactory;
        WebSocketClientInitiationResult& result;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::NotifyingSharedOptional<HttpClientImpl> httpClient;
        HttpClientObserverFactory* httpClientObserverFactory = nullptr;
    public:
        infra::ProxyCreator<decltype(Creators::httpClientInitiationCreator)> initiationClient;
    };

    class WebSocketClientFactorySingleConnection
        : public WebSocketClientConnector
        , private WebSocketClientInitiationResult
    {
    public:
        WebSocketClientFactorySingleConnection(ConnectionFactoryWithNameResolver& connectionFactory,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, const WebSocketClientInitiation::Creators& creators);

        // Implementation of WebSocketClientConnector
        virtual void Connect(WebSocketClientObserverFactory& factory) override;
        virtual void CancelConnect(WebSocketClientObserverFactory& factory) override;

    private:
        // Implementation of WebSocketClientInitiationResult
        virtual void InitiationDone(services::Connection& connection) override;
        virtual void InitiationExpired() override;
        virtual void InitiationError() override;
        virtual void InitiationConnectionFailed(ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason) override;
        virtual void InitiationCancelled() override;

        WebSocketClientObserverFactory::ConnectFailReason Translate(ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason reason);

    private:
        infra::Optional<WebSocketClientInitiation> initiation;
        infra::NotifyingSharedOptional<WebSocketClientConnectionObserver> webSocket;
        ConnectionFactoryWithNameResolver& connectionFactory;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        WebSocketClientInitiation::Creators creators;
    };
}

#endif
