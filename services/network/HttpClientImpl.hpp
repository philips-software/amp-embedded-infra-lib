#ifndef SERVICES_HTTP_CLIENT_IMPL_HPP
#define SERVICES_HTTP_CLIENT_IMPL_HPP

#include "infra/stream/CountingInputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"
#include "services/network/HttpClient.hpp"

namespace services
{
    class HttpClientImpl
        : public ConnectionObserver
        , public HttpClient
        , protected HttpHeaderParserObserver
    {
    public:
        explicit HttpClientImpl(infra::BoundedConstString hostname);

        void Retarget(infra::BoundedConstString hostname);

        // Implementation of HttpClient
        void Get(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Head(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Options(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Post(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Put(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Patch(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void AckReceived() override;
        void CloseConnection() override;
        Connection& GetConnection() override;

        // Implementation of ConnectionObserver
        void Attached() override;
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;
        void Close() override;
        void Detaching() override;

    protected:
        // Implementation of HttpHeaderParserObserver
        void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine) override;
        void HeaderAvailable(HttpHeader header) override;
        void HeaderParsingDone(bool error) override;

        void BodyComplete();

    private:
        void ExpectResponse();
        void HandleData();
        void BodyReceived();
        void BodyReaderDestroyed();
        void Reset();
        bool ReadChunkLength();
        void ExecuteRequest(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers);
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        void AbortAndDestroy();

    private:
        class BodyReader
        {
        public:
            BodyReader(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader, uint32_t contentLength);

            infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
            infra::LimitedStreamReader limitedReader;
            infra::CountingStreamReader countingReader{ limitedReader };
        };

        class SendingState
        {
        public:
            explicit SendingState(HttpClientImpl& client);
            virtual ~SendingState() = default;

            virtual void Activate() = 0;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;

        protected:
            void NextState();

        protected:
            HttpClientImpl& client;
        };

        class SendingStateRequest
            : public SendingState
        {
        public:
            explicit SendingStateRequest(HttpClientImpl& client);

            void Activate() override;
            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        };

        class SendingStateForwardSendStream
            : public SendingState
        {
        public:
            SendingStateForwardSendStream(HttpClientImpl& client);
            SendingStateForwardSendStream(const SendingStateForwardSendStream& other);
            SendingStateForwardSendStream& operator=(const SendingStateForwardSendStream& other) = delete;
            ~SendingStateForwardSendStream() override = default;

            void Activate() override;
            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            class ChunkWriter
                : public infra::LimitedStreamWriter
            {
            public:
                ChunkWriter(SendingStateForwardSendStream& state, infra::SharedPtr<infra::StreamWriter>&& writer);
                ~ChunkWriter();

            private:
                SendingStateForwardSendStream& state;
                infra::SharedPtr<infra::StreamWriter> writer;
                std::size_t start;
            };

        private:
            infra::NotifyingSharedOptional<ChunkWriter> chunkWriter;
            bool first = true;
            bool done = false;
        };

    protected:
        std::optional<HttpRequestFormatter> request;
        std::optional<HttpHeaderParser> response;

    private:
        infra::BoundedConstString hostname;
        HttpStatusCode statusCode = HttpStatusCode::OK;
        std::optional<uint32_t> contentLength;
        bool headerParsingDone = false;
        bool headerParsingError = false;
        bool chunkedEncoding = false;
        bool firstChunk = true;
        std::optional<BodyReader> bodyReader;
        infra::AccessedBySharedPtr bodyReaderAccess;
        infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
        infra::PolymorphicVariant<SendingState, SendingStateRequest, SendingStateForwardSendStream> sendingState;
        infra::PolymorphicVariant<SendingState, SendingStateRequest, SendingStateForwardSendStream> nextState;
    };

    template<class HttpClient = services::HttpClientImpl, class... Args>
    class HttpClientConnectorImpl
        : public services::HttpClientConnector
        , public services::ClientConnectionObserverFactory
    {
    public:
        HttpClientConnectorImpl(services::ConnectionFactory& connectionFactory, services::IPAddress address, Args&&... args);

        void Stop(const infra::Function<void()>& onDone);

        // Implementation of ClientConnectionObserverFactory
        services::IPAddress Address() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

        // Implementation of HttpClientConnector
        void Connect(services::HttpClientObserverFactory& factory) override;
        void CancelConnect(services::HttpClientObserverFactory& factory) override;

    private:
        void TryConnectWaiting();

    private:
        template<std::size_t... I>
        infra::SharedPtr<HttpClient> InvokeEmplace(std::index_sequence<I...>);

    private:
        services::IPAddress address;
        infra::StringOutputStream::WithStorage<47> ipAddress;
        services::ConnectionFactory& connectionFactory;
        infra::NotifyingSharedOptional<HttpClient> client;
        std::tuple<Args...> args;

        services::HttpClientObserverFactory* clientObserverFactory = nullptr;
        infra::IntrusiveList<services::HttpClientObserverFactory> waitingClientObserverFactories;
    };

    template<class HttpClient = HttpClientImpl, class... Args>
    class HttpClientConnectorWithNameResolverImpl
        : public HttpClientConnector
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        HttpClientConnectorWithNameResolverImpl(ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args);

        void Stop(const infra::Function<void()>& onDone);

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        infra::BoundedConstString Hostname() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

        // Implementation of HttpClientConnector
        void Connect(HttpClientObserverFactory& factory) override;
        void CancelConnect(HttpClientObserverFactory& factory) override;

    private:
        void TryConnectWaiting();

    private:
        template<std::size_t... I>
        infra::SharedPtr<HttpClient> InvokeEmplace(std::index_sequence<I...>);

    private:
        ConnectionFactoryWithNameResolver& connectionFactory;
        infra::NotifyingSharedOptional<HttpClient> client;
        std::tuple<Args...> args;

        HttpClientObserverFactory* clientObserverFactory = nullptr;
        infra::IntrusiveList<HttpClientObserverFactory> waitingClientObserverFactories;
    };

    class HttpClientImplWithRedirection
        : public HttpClientImpl
        , private ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        template<std::size_t MaxSize>
        using WithRedirectionUrlSize = infra::WithStorage<HttpClientImplWithRedirection, infra::BoundedString::WithStorage<MaxSize>>;

        HttpClientImplWithRedirection(infra::BoundedString redirectedUrlStorage, infra::BoundedConstString hostname, ConnectionFactoryWithNameResolver& connectionFactory);
        ~HttpClientImplWithRedirection();

        // Implementation of ConnectionObserver
        void Attached() override;
        void Detaching() override;

        // Implementation of HttpClient
        void Get(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Head(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Options(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Post(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Put(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        void Patch(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;

    protected:
        virtual void Redirecting(infra::BoundedConstString url)
        {}

    private:
        // Implementation of HttpClientObserverFactory
        infra::BoundedConstString Hostname() const override;
        uint16_t Port() const override;
        void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> client)>&& createdObserver) override;
        void ConnectionFailed(ConnectFailReason reason) override;

    protected:
        // Implementation of HttpHeaderParserObserver
        void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine) override;
        void HeaderAvailable(HttpHeader header) override;
        void HeaderParsingDone(bool error) override;

    private:
        void Redirect();
        void RedirectFailed();
        std::optional<uint16_t> PortFromScheme(infra::BoundedConstString scheme) const;

    private:
        class Query
        {
        public:
            Query() = default;
            Query(const Query& other) = delete;
            Query& operator=(const Query& other) = delete;
            virtual ~Query() = default;

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) = 0;
        };

        class QueryGet
            : public Query
        {
        public:
            explicit QueryGet(HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryHead
            : public Query
        {
        public:
            explicit QueryHead(HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryConnect
            : public Query
        {
        public:
            explicit QueryConnect(HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryOptions
            : public Query
        {
        public:
            explicit QueryOptions(HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryPost
            : public Query
        {
        public:
            QueryPost(infra::BoundedConstString content, HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            infra::BoundedConstString content;
            HttpHeaders headers;
        };

        class QueryPostChunked
            : public Query
        {
        public:
            explicit QueryPostChunked(HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryPut
            : public Query
        {
        public:
            QueryPut(infra::BoundedConstString content, HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            infra::BoundedConstString content;
            HttpHeaders headers;
        };

        class QueryPutChunked
            : public Query
        {
        public:
            explicit QueryPutChunked(HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryPatch
            : public Query
        {
        public:
            QueryPatch(infra::BoundedConstString content, HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
            infra::BoundedConstString content;
        };

        class QueryPatchChunked
            : public Query
        {
        public:
            explicit QueryPatchChunked(HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryDelete
            : public Query
        {
        public:
            QueryDelete(infra::BoundedConstString content, HttpHeaders headers);

            void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
            infra::BoundedConstString content;
        };

    private:
        infra::BoundedString redirectedUrlStorage;
        ConnectionFactoryWithNameResolver& connectionFactory;
        infra::BoundedConstString redirectedHostname;
        uint16_t redirectedPort;
        infra::BoundedConstString redirectedPath;
        infra::SharedPtr<HttpClientImplWithRedirection> self;
        uint8_t redirectionCount = 0;
        uint8_t maxRedirection = 10;

        bool redirecting = false;
        bool connecting = false;
        std::optional<infra::PolymorphicVariant<Query, QueryGet, QueryHead, QueryConnect, QueryOptions, QueryPost, QueryPostChunked, QueryPut, QueryPutChunked, QueryPatch, QueryPatchChunked, QueryDelete>> query;
    };

    ////    Implementation    ////

    template<class HttpClient, class... Args>
    HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::HttpClientConnectorWithNameResolverImpl(services::ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args)
        : connectionFactory(connectionFactory)
        , client([this]()
              {
                  TryConnectWaiting();
              })
        , args(std::forward<Args>(args)...)
    {}

    template<class HttpClient, class... Args>
    void HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::Stop(const infra::Function<void()>& onDone)
    {
        if (client.Allocatable())
            onDone();
        else
        {
            client.OnAllocatable(onDone);
            if (client)
                client->CloseConnection();
        }
    }

    template<class HttpClient, class... Args>
    infra::BoundedConstString HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::Hostname() const
    {
        return clientObserverFactory->Hostname();
    }

    template<class HttpClient, class... Args>
    uint16_t HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::Port() const
    {
        return clientObserverFactory->Port();
    }

    template<class HttpClient, class... Args>
    template<std::size_t... I>
    infra::SharedPtr<HttpClient> HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::InvokeEmplace(std::index_sequence<I...>)
    {
        return client.Emplace(Hostname(), std::get<I>(args)...);
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(clientObserverFactory != nullptr);
        auto httpClientPtr = InvokeEmplace(std::make_index_sequence<sizeof...(Args)>{});

        clientObserverFactory->ConnectionEstablished([&httpClientPtr, &createdObserver](infra::SharedPtr<HttpClientObserver> observer)
            {
                if (observer)
                {
                    createdObserver(httpClientPtr);
                    httpClientPtr->Attach(observer);
                }
            });

        clientObserverFactory = nullptr;
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectionFailed(ConnectFailReason reason)
    {
        assert(clientObserverFactory != nullptr);

        switch (reason)
        {
            case ConnectFailReason::refused:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::refused);
                break;
            case ConnectFailReason::connectionAllocationFailed:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed);
                break;
            case ConnectFailReason::nameLookupFailed:
                clientObserverFactory->ConnectionFailed(HttpClientObserverFactory::ConnectFailReason::nameLookupFailed);
                break;
            default:
                std::abort();
        }

        clientObserverFactory = nullptr;
        TryConnectWaiting();
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::Connect(HttpClientObserverFactory& factory)
    {
        waitingClientObserverFactories.push_back(factory);
        TryConnectWaiting();
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::CancelConnect(HttpClientObserverFactory& factory)
    {
        if (clientObserverFactory == &factory)
        {
            connectionFactory.CancelConnect(*this);
            clientObserverFactory = nullptr;
        }
        else
            waitingClientObserverFactories.erase(factory);

        TryConnectWaiting();
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::TryConnectWaiting()
    {
        if (clientObserverFactory == nullptr && client.Allocatable() && !waitingClientObserverFactories.empty())
        {
            clientObserverFactory = &waitingClientObserverFactories.front();
            waitingClientObserverFactories.pop_front();
            connectionFactory.Connect(*this);
        }
    }

    template<class HttpClient, class... Args>
    HttpClientConnectorImpl<HttpClient, Args...>::HttpClientConnectorImpl(services::ConnectionFactory& connectionFactory, services::IPAddress address, Args&&... args)
        : address(address)
        , connectionFactory(connectionFactory)
        , client([this]()
              {
                  TryConnectWaiting();
              })
        , args(std::forward<Args>(args)...)
    {}

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::Stop(const infra::Function<void()>& onDone)
    {
        if (client.Allocatable())
            onDone();
        else
        {
            client.OnAllocatable(onDone);
            if (client)
                client->CloseConnection();
        }
    }

    template<class HttpClient, class... Args>
    services::IPAddress HttpClientConnectorImpl<HttpClient, Args...>::Address() const
    {
        return address;
    }

    template<class HttpClient, class... Args>
    uint16_t HttpClientConnectorImpl<HttpClient, Args...>::Port() const
    {
        return 443;
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(clientObserverFactory != nullptr);
        auto httpClientPtr = InvokeEmplace(std::make_index_sequence<sizeof...(Args)>{});

        clientObserverFactory->ConnectionEstablished([&httpClientPtr, &createdObserver](infra::SharedPtr<services::HttpClientObserver> observer)
            {
                if (observer)
                {
                    createdObserver(httpClientPtr);
                    httpClientPtr->Attach(observer);
                }
            });

        clientObserverFactory = nullptr;
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::ConnectionFailed(ConnectFailReason reason)
    {
        assert(clientObserverFactory != nullptr);

        switch (reason)
        {
            case ConnectFailReason::refused:
                clientObserverFactory->ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::refused);
                break;
            case ConnectFailReason::connectionAllocationFailed:
                clientObserverFactory->ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed);
                break;
            default:
                std::abort();
        }

        clientObserverFactory = nullptr;
        TryConnectWaiting();
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::Connect(services::HttpClientObserverFactory& factory)
    {
        waitingClientObserverFactories.push_back(factory);
        TryConnectWaiting();
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::CancelConnect(services::HttpClientObserverFactory& factory)
    {
        if (clientObserverFactory == &factory)
        {
            connectionFactory.CancelConnect(*this);
            clientObserverFactory = nullptr;
        }
        else
            waitingClientObserverFactories.erase(factory);

        TryConnectWaiting();
    }

    template<class HttpClient, class... Args>
    template<std::size_t... I>
    infra::SharedPtr<HttpClient> HttpClientConnectorImpl<HttpClient, Args...>::InvokeEmplace(std::index_sequence<I...>)
    {
        ipAddress.Storage().clear();
        if (address.Is<services::IPv4Address>())
        {
            auto& addr = address.Get<services::IPv4Address>();
            ipAddress << addr[0] << '.' << addr[1] << '.' << addr[2] << '.' << addr[3];
        }
        else
        {
            auto& addr = address.Get<services::IPv6Address>();
            ipAddress << addr[0] << '.' << addr[1] << '.' << addr[2] << '.' << addr[3] << '.' << addr[4] << '.' << addr[5] << '.' << addr[6] << '.' << addr[7];
        }

        return client.Emplace(ipAddress.Storage(), std::get<I>(args)...);
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::TryConnectWaiting()
    {
        if (clientObserverFactory == nullptr && client.Allocatable() && !waitingClientObserverFactories.empty())
        {
            clientObserverFactory = &waitingClientObserverFactories.front();
            waitingClientObserverFactories.pop_front();
            connectionFactory.Connect(*this);
        }
    }

}

#endif
