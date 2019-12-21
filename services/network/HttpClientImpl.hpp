#ifndef SERVICES_HTTP_CLIENT_IMPL_HPP
#define SERVICES_HTTP_CLIENT_IMPL_HPP

#include "infra/util/Optional.hpp"
#include "infra/util/PolymorphicVariant.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/stream/CountingInputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "services/network/HttpClient.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"

namespace services
{
    class HttpRequestFormatter
    {
    public:
        HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers);
        HttpRequestFormatter(HttpVerb verb, infra::BoundedConstString hostname, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers);

        std::size_t Size() const;
        void Write(infra::TextOutputStream stream) const;

    private:
        void AddContentLength(std::size_t size);
        std::size_t HeadersSize() const;

    private:
        HttpVerb verb;
        infra::BoundedConstString requestTarget;
        infra::BoundedConstString content;
        infra::BoundedString::WithStorage<8> contentLength;
        infra::Optional<HttpHeader> contentLengthHeader;
        HttpHeader hostHeader;
        const HttpHeaders headers;
    };

    class HttpClientImpl
        : public ConnectionObserver
        , public HttpClient
    {
    public:
        HttpClientImpl(infra::BoundedConstString hostname);

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
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void AckReceived() override;
        virtual void Close() override;
        virtual Connection& GetConnection() override;

        // Implementation of ConnectionObserver
        virtual void Attached(Connection& connection) override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void Detaching() override;

    protected:
        virtual void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine);

    private:
        void ExpectResponse();
        void HandleData();
        void BodyReceived();
        void BodyReaderDestroyed();
        void BodyComplete();
        void ExecuteRequest(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers);
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers);
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        uint32_t ReadContentSizeFromObserver() const;
        void AbortAndDestroy();

    private:
        class HttpResponseParser
        {
        public:
            HttpResponseParser(HttpClientImpl& httpClient);

            void DataReceived(infra::StreamReaderWithRewinding& reader);
            bool Done() const;
            bool Error() const;
            uint32_t ContentLength() const;

        private:
            void ParseStatusLine(infra::StreamReaderWithRewinding& reader);
            bool HttpVersionValid(infra::BoundedConstString httpVersion);

            void ParseHeaders(infra::StreamReaderWithRewinding& reader);
            HttpHeader HeaderFromString(infra::BoundedConstString header);

            void SetError();

        private:
            HttpClientImpl& httpClient;
            bool done = false;
            bool error = false;
            bool statusParsed = false;
            HttpStatusCode statusCode;
            infra::Optional<uint32_t> contentLength;
        };

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
            SendingStateForwardSendStream(const SendingStateForwardSendStream& other);
            SendingStateForwardSendStream(HttpClientImpl& client, std::size_t contentSize);

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
        infra::Optional<HttpResponseParser> response;

    private:
        infra::BoundedConstString hostname;
        infra::Optional<uint32_t> contentLength;
        infra::Optional<BodyReader> bodyReader;
        infra::AccessedBySharedPtr bodyReaderAccess;
        infra::PolymorphicVariant<SendingState, SendingStateRequest, SendingStateForwardSendStream, SendingStateForwardFillContent> sendingState;
        infra::PolymorphicVariant<SendingState, SendingStateRequest, SendingStateForwardSendStream, SendingStateForwardFillContent> nextState;
    };

    template<class HttpClient = HttpClientImpl, class... Args>
    class HttpClientConnectorImpl
        : public HttpClientConnector
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        HttpClientConnectorImpl(ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args);

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

    ////    Implementation    ////

    template<class HttpClient, class... Args>
    HttpClientConnectorImpl<HttpClient, Args...>::HttpClientConnectorImpl(services::ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args)
        : connectionFactory(connectionFactory)
        , client([this]() { TryConnectWaiting(); })
        , args(std::forward<Args>(args)...)
    {}

    template<class HttpClient, class... Args>
    infra::BoundedConstString HttpClientConnectorImpl<HttpClient, Args...>::Hostname() const
    {
        return clientObserverFactory->Hostname();
    }

    template<class HttpClient, class... Args>
    uint16_t HttpClientConnectorImpl<HttpClient, Args...>::Port() const
    {
        return clientObserverFactory->Port();
    }

    template<class HttpClient, class... Args>
    template<std::size_t... I>
    infra::SharedPtr<HttpClient> HttpClientConnectorImpl<HttpClient, Args...>::InvokeEmplace(infra::IndexSequence<I...>)
    {
        return client.Emplace(Hostname(), std::get<I>(args)...);
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(clientObserverFactory != nullptr);
        auto httpClientPtr = InvokeEmplace(infra::MakeIndexSequence<sizeof...(Args)>{});

        clientObserverFactory->ConnectionEstablished([&httpClientPtr, &createdObserver](infra::SharedPtr<HttpClientObserver> observer)
        {
            if (observer)
            {
                httpClientPtr->Attach(observer);
                createdObserver(httpClientPtr);
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
    void HttpClientConnectorImpl<HttpClient, Args...>::Connect(HttpClientObserverFactory& factory)
    {
        waitingClientObserverFactories.push_back(factory);
        TryConnectWaiting();
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::CancelConnect(HttpClientObserverFactory& factory)
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
