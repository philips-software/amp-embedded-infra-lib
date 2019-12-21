#ifndef DI_COMM_HTTP_SERVER_HPP
#define DI_COMM_HTTP_SERVER_HPP

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
        HttpResponseHeaderBuilder(infra::TextOutputStream& output);
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
        HttpResponse(std::size_t maxBodySize);
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

    private:
        std::size_t maxBodySize;
    };

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
        virtual void RespondToRequest(HttpRequestParser& parser, HttpServerConnection& connection) = 0;
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
        virtual void Connected() override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;
        virtual void Detaching() override;
        virtual void Close() override;
        virtual void Abort() override;

        // Implementation of HttpServerConnection
        virtual void SendResponse(const HttpResponse& response) override;
        virtual void SendResponseWithoutNextRequest(const HttpResponse& response) override;
        virtual void TakeOverConnection(ConnectionObserver& newObserver) override;

    protected:
        virtual void SendingHttpResponse(infra::BoundedConstString response) {}
        virtual void ReceivedHttpRequest(infra::BoundedConstString request) {}

        virtual void SetIdle();

        // Implementation of HttpPageServer
        virtual HttpPage* PageForRequest(const HttpRequestParser& request) override;

    private:
        void ReceivedTooMuchData(infra::TextInputStream& receiveStream);
        void ReceivedRequest(infra::TextInputStream& receiveStream, std::size_t available);
        void TryHandleRequest(HttpRequestParser& request);
        void HandleRequest(HttpRequestParser& request);
        void RequestIsNowInProgress();
        void ServePage(HttpRequestParser& request);
        void RequestSendStream();
        void PrepareForNextRequest();
        bool Expect100(HttpRequestParser& request) const;
        void SendBuffer();
        void CheckIdleClose();

    protected:
        infra::SharedPtr<infra::StreamWriter> streamWriter;

    protected:
        HttpPageServer& httpServer;
        Connection* connection = nullptr;
        bool send100Response = false;
        infra::BoundedString& buffer;
        bool closeWhenIdle = false;
        bool idle = false;
        bool requestInProgress = false;
        bool sendingResponse = false;
        infra::TimerSingleShot initialIdle;
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
