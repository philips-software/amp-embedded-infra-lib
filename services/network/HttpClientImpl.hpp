#ifndef SERVICES_HTTP_CLIENT_IMPL_HPP
#define SERVICES_HTTP_CLIENT_IMPL_HPP

#include "infra/util/Optional.hpp"
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
        template<std::size_t MaxHeaderSize>
            using WithMaxHeaderSize = infra::WithStorage<HttpClientImpl, infra::BoundedString::WithStorage<MaxHeaderSize>>;

        HttpClientImpl(infra::BoundedString& headerBuffer, infra::BoundedConstString hostname);

        void AttachObserver(const infra::SharedPtr<HttpClientObserver>& observer);

        // Implementation of HttpClient
        virtual void Get(infra::BoundedConstString requestTarget, HttpHeaders headers) override;
        virtual void Head(infra::BoundedConstString requestTarget, HttpHeaders headers) override;
        virtual void Connect(infra::BoundedConstString requestTarget, HttpHeaders headers) override;
        virtual void Options(infra::BoundedConstString requestTarget, HttpHeaders headers) override;
        virtual void Post(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers) override;
        virtual void Put(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Put(infra::BoundedConstString requestTarget, std::size_t contentSize, HttpHeaders headers = noHeaders) override;
        virtual void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;

        virtual void AckReceived() override;
        virtual void Close() override;

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void Connected() override;
        virtual void ClosingConnection() override;

    protected:
        virtual void StatusAvailable(HttpStatusCode code, infra::BoundedConstString statusLine);

    private:
        void ForwardStream(infra::SharedPtr<infra::StreamWriter>&& writer);
        void WriteRequest(infra::SharedPtr<infra::StreamWriter>&& writer);
        void RequestForwardStreamOrExpectResponse();
        void HandleData();
        void BodyReceived();
        void BodyReaderDestroyed();
        void BodyComplete();
        void ExecuteRequest(HttpVerb verb, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers);
        void ExecuteRequestWithContent(HttpVerb verb, infra::BoundedConstString requestTarget, std::size_t contentSize, const HttpHeaders headers);
        void AbortAndDestroy();

    private:
        class HttpResponseParser
        {
        public:
            HttpResponseParser(HttpClientImpl& httpClient, infra::BoundedString& headerBuffer);

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
            infra::BoundedString& headerBuffer;
            bool done = false;
            bool error = false;
            bool statusParsed = false;
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

    protected:
        infra::SharedPtr<HttpClientObserver> observer;
        infra::Optional<HttpRequestFormatter> request;
        infra::Optional<HttpResponseParser> response;

    private:
        infra::BoundedString& headerBuffer;
        infra::BoundedConstString hostname;
        infra::Optional<uint32_t> contentLength;
        infra::Optional<BodyReader> bodyReader;
        infra::AccessedBySharedPtr bodyReaderAccess;
        std::size_t streamingContentSize = 0;
        bool forwardingStream = false;
        infra::AccessedBySharedPtr forwardStreamAccess;
        infra::SharedPtr<infra::StreamWriter> forwardStreamPtr;
    };

    template<class HttpClient = HttpClientImpl, class... Args>
    class HttpClientConnectorImpl
        : public HttpClientConnector
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        template<std::size_t MaxHeaderSize>
            using WithMaxHeaderSize = infra::WithStorage<HttpClientConnectorImpl, infra::BoundedString::WithStorage<MaxHeaderSize>>;

        HttpClientConnectorImpl(infra::BoundedString& headerBuffer, ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args);

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
        template<class T, T... Ints>
        class integer_sequence
        {};

        template<std::size_t... I>
            using index_sequence = integer_sequence<std::size_t, I...>;

        template<std::size_t... Ns>
        struct make_integer_sequence_helper;

        template<std::size_t I, std::size_t... Ns>
        struct make_integer_sequence_helper<I, Ns...>
        {
            using type = typename make_integer_sequence_helper<I - 1, I - 1, Ns...>::type;
        };

        template<std::size_t... Ns>
        struct make_integer_sequence_helper<0, Ns...>
        {
            using type = integer_sequence<std::size_t, Ns...>;
        };

        template<std::size_t N>
            using make_integer_sequence = typename make_integer_sequence_helper<N>::type;

        template<std::size_t N>
            using make_index_sequence = make_integer_sequence<N>;

        template<std::size_t... I>
            infra::SharedPtr<HttpClient> InvokeEmplace(index_sequence<I...>);

    private:
        infra::BoundedString& headerBuffer;
        ConnectionFactoryWithNameResolver& connectionFactory;
        infra::NotifyingSharedOptional<HttpClient> client;
        std::tuple<Args...> args;

        HttpClientObserverFactory* clientObserverFactory = nullptr;
        infra::IntrusiveList<HttpClientObserverFactory> waitingClientObserverFactories;
    };

    ////    Implementation    ////

    template<class HttpClient, class... Args>
    HttpClientConnectorImpl<HttpClient, Args...>::HttpClientConnectorImpl(infra::BoundedString& headerBuffer, services::ConnectionFactoryWithNameResolver& connectionFactory, Args&&... args)
        : headerBuffer(headerBuffer)
        , connectionFactory(connectionFactory)
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
    infra::SharedPtr<HttpClient> HttpClientConnectorImpl<HttpClient, Args...>::InvokeEmplace(index_sequence<I...>)
    {
        return client.Emplace(headerBuffer, Hostname(), std::get<I>(args)...);
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::ConnectionEstablished(infra::AutoResetFunction<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)>&& createdObserver)
    {
        assert(clientObserverFactory);
        auto clientPtr = InvokeEmplace(make_index_sequence<sizeof...(Args)>{});

        clientObserverFactory->ConnectionEstablished([&clientPtr, &createdObserver](infra::SharedPtr<HttpClientObserver> observer)
        {
            if (observer)
            {
                clientPtr->AttachObserver(observer);
                createdObserver(clientPtr);
            }
        });

        clientObserverFactory = nullptr;
    }

    template<class HttpClient, class... Args>
    void HttpClientConnectorImpl<HttpClient, Args...>::ConnectionFailed(ConnectFailReason reason)
    {
        assert(clientObserverFactory);

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
