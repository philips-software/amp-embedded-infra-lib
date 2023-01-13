#ifndef SERVICES_HTTP_CLIENT_IMPL_HPP
#define SERVICES_HTTP_CLIENT_IMPL_HPP

#include "infra/util/Optional.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/stream/CountingInputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "services/network/HttpClient.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"

namespace services
{
    class HttpClientImpl
        : public ConnectionObserver
        , public HttpClient
        , protected HttpHeaderParserObserver
    {
    public:
        HttpClientImpl(infra::BoundedConstString hostname);

        void Retarget(infra::BoundedConstString hostname);

        // Implementation of HttpClient
        virtual void Get(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Head(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Options(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Patch(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void AckReceived() override;
        virtual void CloseConnection() override;
        virtual Connection& GetConnection() override;

        // Implementation of ConnectionObserver
        virtual void Attached() override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void Close() override;
        virtual void Detaching() override;

    protected:
        // Implementation of HttpHeaderParserObserver
        virtual void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine) override;
        virtual void HeaderAvailable(HttpHeader header) override;
        virtual void HeaderParsingDone(bool error) override;

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
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers);
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        uint32_t ReadContentSizeFromObserver() const;
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
            SendingState(HttpClientImpl& client);
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
            SendingStateRequest(HttpClientImpl& client);

            virtual void Activate() override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        };

        class SendingStateForwardSendStream
            : public SendingState
        {
        public:
            SendingStateForwardSendStream(HttpClientImpl& client, std::size_t contentSize);
            SendingStateForwardSendStream(const SendingStateForwardSendStream& other);
            SendingStateForwardSendStream& operator=(const SendingStateForwardSendStream& other) = default;
            ~SendingStateForwardSendStream() = default;

            virtual void Activate() override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            std::size_t contentSize;
            infra::AccessedBySharedPtr forwardStreamAccess;
            infra::SharedPtr<infra::StreamWriter> forwardStreamPtr;
        };

        class SendingStateForwardFillContent
            : public SendingState
        {
        public:
            SendingStateForwardFillContent(HttpClientImpl& client, std::size_t contentSize);

            virtual void Activate() override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        private:
            class WindowWriter
                : public infra::StreamWriter
            {
            public:
                WindowWriter(infra::StreamWriter& writer, std::size_t start, std::size_t limit);

                std::size_t Processed() const;

                virtual void Insert(infra::ConstByteRange range, infra::StreamErrorPolicy& errorPolicy) override;
                virtual std::size_t Available() const override;

            private:
                infra::StreamWriter& writer;
                std::size_t start;
                std::size_t limit;
                std::size_t processed = 0;
            };

        private:
            std::size_t contentSize;
            std::size_t processed = 0;
        };

    protected:
        infra::Optional<HttpRequestFormatter> request;
        infra::Optional<HttpHeaderParser> response;

    private:
        infra::BoundedConstString hostname;
        HttpStatusCode statusCode = HttpStatusCode::OK;
        infra::Optional<uint32_t> contentLength;
        bool headerParsingDone = false;
        bool headerParsingError = false;
        bool chunkedEncoding = false;
        bool firstChunk = true;
        infra::Optional<BodyReader> bodyReader;
        infra::AccessedBySharedPtr bodyReaderAccess;
        infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
        infra::PolymorphicVariant<SendingState, SendingStateRequest, SendingStateForwardSendStream, SendingStateForwardFillContent> sendingState;
        infra::PolymorphicVariant<SendingState, SendingStateRequest, SendingStateForwardSendStream, SendingStateForwardFillContent> nextState;
    };

    template<class HttpClient = services::HttpClientImpl, class... Args>
    class HttpClientConnectorImpl
        : public services::HttpClientConnector
        , public services::ClientConnectionObserverFactory
    {
    public:
        HttpClientConnectorImpl(services::ConnectionFactory& connectionFactory, services::IPAddress address, Args&&... args);

        // Implementation of ClientConnectionObserverFactory
        virtual services::IPAddress Address() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

        // Implementation of HttpClientConnector
        virtual void Connect(services::HttpClientObserverFactory& factory) override;
        virtual void CancelConnect(services::HttpClientObserverFactory& factory) override;

    private:
        void TryConnectWaiting();

    private:
        template<std::size_t... I>
        infra::SharedPtr<HttpClient> InvokeEmplace(infra::IndexSequence<I...>);

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

        // Implementation of ClientConnectionObserverFactoryWithNameResolver
        virtual infra::BoundedConstString Hostname() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> connectionObserver)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

        // Implementation of HttpClientConnector
        virtual void Connect(HttpClientObserverFactory& factory) override;
        virtual void CancelConnect(HttpClientObserverFactory& factory) override;

    private:
        void TryConnectWaiting();

    private:
        template<std::size_t... I>
            infra::SharedPtr<HttpClient> InvokeEmplace(infra::IndexSequence<I...>);

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
        virtual void Attached() override;
        virtual void Detaching() override;

        // Implementation of HttpClient
        virtual void Get(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Head(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Options(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) override;
        virtual void Post(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Patch(infra::BoundedConstString requestTarget, HttpHeaders headers = noHeaders) override;
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;

    protected:
        virtual void Redirecting(infra::BoundedConstString url) {}

    private:
        // Implementation of HttpClientObserverFactory
        virtual infra::BoundedConstString Hostname() const override;
        virtual uint16_t Port() const override;
        virtual void ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<ConnectionObserver> client)>&& createdObserver) override;
        virtual void ConnectionFailed(ConnectFailReason reason) override;

    protected:
        // Implementation of HttpHeaderParserObserver
        virtual void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine) override;
        virtual void HeaderAvailable(HttpHeader header) override;
        virtual void HeaderParsingDone(bool error) override;

    private:
        void Redirect();
        void RedirectFailed();
        infra::Optional<uint16_t> PortFromScheme(infra::BoundedConstString scheme) const;

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
            QueryGet(HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryHead
            : public Query
        {
        public:
            QueryHead(HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryConnect
            : public Query
        {
        public:
            QueryConnect(HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryOptions
            : public Query
        {
        public:
            QueryOptions(HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryPost1
            : public Query
        {
        public:
            QueryPost1(infra::BoundedConstString content, HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            infra::BoundedConstString content;
            HttpHeaders headers;
        };

        class QueryPost2
            : public Query
        {
        public:
            QueryPost2(std::size_t contentSize, HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            std::size_t contentSize;
            HttpHeaders headers;
        };

        class QueryPost3
            : public Query
        {
        public:
            QueryPost3(HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryPut1
            : public Query
        {
        public:
            QueryPut1(infra::BoundedConstString content, HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            infra::BoundedConstString content;
            HttpHeaders headers;
        };

        class QueryPut2
            : public Query
        {
        public:
            QueryPut2(std::size_t contentSize, HttpHeaders headers);

            virtual void Execute(HttpClient & client, infra::BoundedConstString requestTarget) override;

        private:
            std::size_t contentSize;
            HttpHeaders headers;
        };

        class QueryPut3
            : public Query
        {
        public:
            QueryPut3(HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryPatch1
            : public Query
        {
        public:
            QueryPatch1(infra::BoundedConstString content, HttpHeaders headers);

            virtual void Execute(HttpClient & client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
            infra::BoundedConstString content;
        };

        class QueryPatch2
            : public Query
        {
        public:
            QueryPatch2(HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

        private:
            HttpHeaders headers;
        };

        class QueryDelete
            : public Query
        {
        public:
            QueryDelete(infra::BoundedConstString content, HttpHeaders headers);

            virtual void Execute(HttpClient& client, infra::BoundedConstString requestTarget) override;

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
        infra::Optional<infra::PolymorphicVariant<Query, QueryGet, QueryHead, QueryConnect, QueryOptions, QueryPost1, QueryPost2, QueryPost3, QueryPut1, QueryPut2, QueryPut3, QueryPatch1, QueryPatch2, QueryDelete>> query;
    };

    ////    Implementation    ////

    template<class HttpClient, class... Args>
    HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::HttpClientConnectorWithNameResolverImpl(services::ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args)
        : connectionFactory(connectionFactory)
        , client([this]() { TryConnectWaiting(); })
        , args(std::forward<Args>(args)...)
    {}

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
    infra::SharedPtr<HttpClient> HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::InvokeEmplace(infra::IndexSequence<I...>)
    {
        return client.Emplace(Hostname(), std::get<I>(args)...);
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorWithNameResolverImpl<HttpClient, Args...>::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(clientObserverFactory != nullptr);
        auto httpClientPtr = InvokeEmplace(infra::MakeIndexSequence<sizeof...(Args)>{});

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
        , client([this]() { TryConnectWaiting(); })
        , args(std::forward<Args>(args)...)
    {}

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
        auto httpClientPtr = InvokeEmplace(infra::MakeIndexSequence<sizeof...(Args)>{});

        clientObserverFactory->ConnectionEstablished([&httpClientPtr, &createdObserver](infra::SharedPtr<services::HttpClientObserver> observer) {
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
    infra::SharedPtr<HttpClient> HttpClientConnectorImpl<HttpClient, Args...>::InvokeEmplace(infra::IndexSequence<I...>)
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
