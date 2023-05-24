#ifndef SERVICES_HTTP_SERVER_HPP
#define SERVICES_HTTP_SERVER_HPP

#include "infra/stream/CountingInputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/IntrusiveForwardList.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/WithStorage.hpp"
#include "services/network/Connection.hpp"
#include "services/network/HttpRequestParser.hpp"
#include "services/network/SingleConnectionListener.hpp"

namespace services
{
    class HttpPage;

    class HttpPageServer
    {
    public:
        HttpPageServer() = default;
        HttpPageServer(const HttpPageServer& other) = delete;
        HttpPageServer& operator=(const HttpPageServer& other) = delete;
        ~HttpPageServer() = default;

    public:
        virtual HttpPage* PageForRequest(const HttpRequestParser& request);
        void AddPage(services::HttpPage& page);

    private:
        infra::IntrusiveForwardList<services::HttpPage> pages;
    };

    class HttpResponseHeaderBuilder
    {
    public:
        explicit HttpResponseHeaderBuilder(infra::TextOutputStream& output);
        HttpResponseHeaderBuilder(infra::TextOutputStream& output, infra::BoundedConstString result);

        void AddHeader(infra::BoundedConstString key, infra::BoundedConstString value);
        void AddHeader(infra::BoundedConstString key, uint32_t value);
        void AddHeader(infra::BoundedConstString key);
        void StartBody();

        infra::TextOutputStream& Stream();

    private:
        infra::TextOutputStream& output;
    };

    class HttpResponse
    {
    protected:
        HttpResponse() = default;
        HttpResponse(const HttpResponse& other) = delete;
        HttpResponse& operator=(const HttpResponse& other) = delete;
        ~HttpResponse() = default;

    public:
        void WriteResponse(infra::TextOutputStream& stream) const;

    public: // For test access
        virtual infra::BoundedConstString Status() const = 0;
        virtual void WriteBody(infra::TextOutputStream& stream) const = 0;
        virtual infra::BoundedConstString ContentType() const;
        virtual void AddHeaders(HttpResponseHeaderBuilder& builder) const;
    };

    class SimpleHttpResponse
        : public HttpResponse
    {
    public:
        SimpleHttpResponse(infra::BoundedConstString status, infra::BoundedConstString body = infra::BoundedConstString());

        infra::BoundedConstString Status() const override;
        void WriteBody(infra::TextOutputStream& stream) const override;
        infra::BoundedConstString ContentType() const override;

    private:
        infra::BoundedConstString status;
        infra::BoundedConstString body;
    };

    extern SimpleHttpResponse httpResponseOk;

    class HttpServerConnection
    {
    protected:
        HttpServerConnection() = default;
        HttpServerConnection(const HttpServerConnection& other) = delete;
        HttpServerConnection& operator=(const HttpServerConnection& other) = delete;
        ~HttpServerConnection() = default;

    public:
        virtual void SendResponse(const HttpResponse& response) = 0;
        virtual void SendResponseWithoutNextRequest(const HttpResponse& response) = 0;
        virtual void TakeOverConnection(ConnectionObserver& observer) = 0;
    };

    class HttpPage
        : public infra::IntrusiveForwardList<HttpPage>::NodeType
    {
    protected:
        HttpPage() = default;
        HttpPage(const HttpPage& other) = delete;
        HttpPage& operator=(const HttpPage& other) = delete;
        ~HttpPage() = default;

    public:
        virtual bool ServesRequest(const infra::Tokenizer& pathTokens) const = 0;
        virtual void RequestReceived(HttpRequestParser& parser, HttpServerConnection& connection) = 0;
        virtual void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) = 0;
        virtual void Close() = 0;
    };

    class SimpleHttpPage
        : public HttpPage
    {
    public:
        void RequestReceived(HttpRequestParser& parser, HttpServerConnection& connection) override;
        void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        void Close() override;

        virtual void RespondToRequest(HttpRequestParser& parser, HttpServerConnection& connection) = 0;

    private:
        HttpServerConnection* connection = nullptr;
        HttpRequestParser* parser = nullptr;
    };

    class HttpPageWithContent
        : public SimpleHttpPage
        , protected SimpleHttpResponse
    {
    public:
        HttpPageWithContent(infra::BoundedConstString path, infra::BoundedConstString body, infra::BoundedConstString contentType);

        // Implementation of SimpleHttpPage
        bool ServesRequest(const infra::Tokenizer& pathTokens) const override;
        void RespondToRequest(HttpRequestParser& parser, HttpServerConnection& connection) override;

        // Implementation of SimpleHttpResponse
        infra::BoundedConstString ContentType() const override;

    private:
        infra::BoundedConstString path;
        infra::BoundedConstString contentType;
    };

    class HttpServerConnectionObserver
        : public ConnectionObserver
        , public HttpServerConnection
        , public HttpPageServer
    {
    public:
        template<::size_t BufferSize>
        using WithBuffer = infra::WithStorage<HttpServerConnectionObserver, infra::BoundedString::WithStorage<BufferSize>>;

        HttpServerConnectionObserver(infra::BoundedString& buffer, HttpPageServer& httpServer);

        // Implementation of ConnectionObserver
        void Attached() override;
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        void DataReceived() override;
        void Detaching() override;
        void Close() override;
        void Abort() override;

        // Implementation of HttpServerConnection
        void SendResponse(const HttpResponse& response) override;
        void SendResponseWithoutNextRequest(const HttpResponse& response) override;
        void TakeOverConnection(ConnectionObserver& newObserver) override;

    protected:
        virtual void SendingHttpResponse(infra::BoundedConstString response)
        {}

        virtual void ReceivedHttpRequest(infra::BoundedConstString request)
        {}

        virtual void SetIdle();
        virtual void RequestIsNowInProgress();

        // Implementation of HttpPageServer
        HttpPage* PageForRequest(const HttpRequestParser& request) override;

    private:
        void ReceivedTooMuchData(infra::StreamReader& reader);
        void ReceivedRequest(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);
        void TryHandleRequest(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);
        void HandleRequest(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);
        void ServePage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);
        void DataReceivedForPage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);
        void PageReaderClosed();
        void RequestSendStream();
        void PrepareForNextRequest();
        bool Expect100() const;
        void SendBuffer();
        void CheckIdleClose();

    protected:
        infra::SharedPtr<infra::StreamWriter> streamWriter;
        infra::SharedPtr<infra::StreamReaderWithRewinding>* readerPtr = nullptr;

    protected:
        infra::BoundedString& buffer;
        HttpPageServer& httpServer;
        Connection* connection = nullptr;
        bool send100Response = false;
        bool closeWhenIdle = false;
        bool idle = false;
        infra::Optional<uint32_t> contentLength;
        uint32_t lengthRead = 0;
        HttpPage* pageServer = nullptr;
        infra::SharedPtr<infra::StreamReaderWithRewinding> pageReader;
        infra::Optional<infra::CountingStreamReaderWithRewinding> pageCountingReader;
        infra::NotifyingSharedOptional<infra::LimitedStreamReaderWithRewinding> pageLimitedReader;
        infra::SharedPtr<void> keepSelfAlive;
        infra::Optional<HttpRequestParserImpl> parser;
        infra::TimerSingleShot initialIdle;
        bool sendingResponse = false;

        friend class SimpleHttpPage;
    };

    class DefaultHttpServer
        : public services::SingleConnectionListener
        , public services::HttpPageServer
    {
    public:
        template<::size_t BufferSize>
        using WithBuffer = infra::WithStorage<DefaultHttpServer, infra::BoundedString::WithStorage<BufferSize>>;

        DefaultHttpServer(infra::BoundedString& buffer, ConnectionFactory& connectionFactory, uint16_t port);

    private:
        infra::BoundedString& buffer;
        infra::Creator<services::ConnectionObserver, HttpServerConnectionObserver, void(IPAddress address)> connectionCreator;
    };
}

#endif
