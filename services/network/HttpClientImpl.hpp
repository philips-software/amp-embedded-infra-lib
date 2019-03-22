#ifndef SERVICES_HTTP_CLIENT_IMPL_HPP
#define SERVICES_HTTP_CLIENT_IMPL_HPP

#include "infra/util/Optional.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "services/network/Http.hpp"
#include "services/network/ConnectionFactoryWithNameResolver.hpp"

namespace services
{
    class HttpRequestFormatter
    {
    public:
        HttpRequestFormatter(infra::BoundedConstString hostname, infra::BoundedConstString method, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        HttpRequestFormatter(infra::BoundedConstString hostname, infra::BoundedConstString method, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers);

        std::size_t Size() const;
        void Write(infra::TextOutputStream stream) const;

    private:
        std::size_t HeadersSize() const;

    private:
        infra::BoundedConstString method;
        infra::BoundedConstString requestTarget;
        infra::BoundedConstString content;
        infra::StringOutputStream::WithStorage<8> contentLength;
        infra::Optional<HttpHeader> contentLengthHeader;
        HttpHeader hostHeader;
        const HttpHeaders headers;
    };

    class HttpResponseParser
    {
    public:
        HttpResponseParser(infra::SharedPtr<HttpClientObserver> observer, infra::BoundedString& headerBuffer);

        void DataReceived(infra::StreamReaderWithRewinding& reader);
        bool Done() const;
        bool Error() const;
        uint32_t ContentLength() const;

    private:
        void ParseStatusLine(infra::StreamReaderWithRewinding& reader);
        bool HttpVersionValid(infra::BoundedConstString httpVersion);
        infra::Optional<HttpStatusCode> StatusCodeFromString(infra::BoundedConstString statusCode);

        void ParseHeaders(infra::StreamReaderWithRewinding& reader);
        HttpHeader HeaderFromString(infra::BoundedConstString header);

        void SetError();

    private:
        infra::SharedPtr<HttpClientObserver> observer;
        infra::BoundedString& headerBuffer;
        bool done = false;
        bool error = false;
        bool statusParsed = false;
        infra::Optional<uint32_t> contentLength;
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
        virtual void Patch(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;
        virtual void Delete(infra::BoundedConstString requestTarget, infra::BoundedConstString content, HttpHeaders headers = noHeaders) override;

        virtual void AckReceived() override;
        virtual void Close() override;

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void Connected() override;
        virtual void ClosingConnection() override;

    private:
        void ExecuteRequest(infra::BoundedConstString method, infra::BoundedConstString requestTarget, const HttpHeaders headers);
        void ExecuteRequestWithContent(infra::BoundedConstString method, infra::BoundedConstString requestTarget, infra::BoundedConstString content, const HttpHeaders headers);

    protected:
        infra::SharedPtr<HttpClientObserver> observer;
        infra::Optional<HttpRequestFormatter> request;
        infra::Optional<HttpResponseParser> response;

    private:
        infra::BoundedString& headerBuffer;
        infra::BoundedConstString hostname;
        infra::Optional<uint32_t> contentLength;
    };

    class HttpClientConnectorImpl
        : public HttpClientConnector
        , public ClientConnectionObserverFactoryWithNameResolver
    {
    public:
        template<std::size_t MaxHeaderSize>
            using WithMaxHeaderSize = infra::WithStorage<HttpClientConnectorImpl, infra::BoundedString::WithStorage<MaxHeaderSize>>;

        HttpClientConnectorImpl(infra::BoundedString& headerBuffer, ConnectionFactoryWithNameResolver& connectionFactory);

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
        infra::BoundedString& headerBuffer;
        ConnectionFactoryWithNameResolver& connectionFactory;
        infra::NotifyingSharedOptional<HttpClientImpl> client;

        HttpClientObserverFactory* clientObserverFactory = nullptr;
        infra::IntrusiveList<HttpClientObserverFactory> waitingClientObserverFactories;
    };
}

#endif
