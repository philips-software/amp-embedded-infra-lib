#ifndef SERVICES_HTTP_PAGE_WEB_SOCKET_HPP
#define SERVICES_HTTP_PAGE_WEB_SOCKET_HPP

#include "services/network/HttpServer.hpp"
#include "services/network/WebSocket.hpp"

namespace services
{
    class HttpPageWebSocket
        : public services::HttpPage
        , public services::ConnectionObserver
        , protected services::HttpResponse
    {
    public:
        HttpPageWebSocket(infra::BoundedConstString path, WebSocketObserverFactory& webSocketObserverFactory);

    public:
        // Implementation of HttpPage
        virtual bool ServesRequest(const infra::Tokenizer& pathTokens) const override;
        virtual void RespondToRequest(services::HttpRequestParser& parser, services::HttpServerConnection& connection) override;

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& stream) override;
        virtual void DataReceived() override;
        virtual void Detaching() override;

    protected:
        // implementation of HttpResponse
        virtual infra::BoundedConstString Status() const override;
        virtual void WriteBody(infra::TextOutputStream& stream) const override;
        virtual void AddHeaders(HttpResponseHeaderBuilder& builder) const override;

    private:
        infra::BoundedConstString path;
        WebSocketObserverFactory& webSocketObserverFactory;
        static const uint8_t MaxWebSocketKeySize = 64;
        infra::BoundedString::WithStorage<MaxWebSocketKeySize> webSocketKey;
    };
}

#endif
