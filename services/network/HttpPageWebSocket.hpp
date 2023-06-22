#ifndef SERVICES_HTTP_PAGE_WEB_SOCKET_HPP
#define SERVICES_HTTP_PAGE_WEB_SOCKET_HPP

#include "services/network/HttpServer.hpp"
#include "services/network/WebSocket.hpp"

namespace services
{
    class HttpPageWebSocket
        : public services::SimpleHttpPage
        , public services::ConnectionObserver
        , protected services::HttpResponse
    {
    public:
        HttpPageWebSocket(infra::BoundedConstString path, WebSocketObserverFactory& webSocketObserverFactory);

    public:
        // Implementation of SimpleHttpPage
        bool ServesRequest(const infra::Tokenizer& pathTokens) const override;
        void RespondToRequest(services::HttpRequestParser& parser, services::HttpServerConnection& connection) override;

        // Implementation of ConnectionObserver
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& stream) override;
        void DataReceived() override;
        void Detaching() override;

    protected:
        // implementation of HttpResponse
        infra::BoundedConstString Status() const override;
        void WriteBody(infra::TextOutputStream& stream) const override;
        void AddHeaders(HttpResponseHeaderBuilder& builder) const override;

    private:
        infra::BoundedConstString path;
        WebSocketObserverFactory& webSocketObserverFactory;
        static const uint8_t MaxWebSocketKeySize = 64;
        infra::BoundedString::WithStorage<MaxWebSocketKeySize> webSocketKey;
    };
}

#endif
