#ifndef SERVICES_HTTP_PAGE_WEB_SOCKET_HPP
#define SERVICES_HTTP_PAGE_WEB_SOCKET_HPP

#include "services/network/HttpServer.hpp"
#include "services/network/WebSocket.hpp"

namespace services
{
    class HttpPageWebSocket
        : public services::HttpPage
        , public services::ConnectionObserver
    {
    public:
        struct Address
        {
            services::IPAddress address;
        };

        HttpPageWebSocket(infra::BoundedConstString path, WebSocketObserverFactory& webSocketObserverFactory, Address address);

    public:
        // Implementation of HttpPage
        virtual bool ServesRequest(const infra::Tokenizer& pathTokens) const override;
        virtual void RespondToRequest(services::HttpRequestParser& parser, services::HttpServerConnection& connection) override;

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& stream) override;
        virtual void DataReceived() override;
        virtual void ClosingConnection() override;

    private:
        infra::BoundedConstString path;
        WebSocketObserverFactory& webSocketObserverFactory;
        services::IPAddress address;
    };
}

#endif
