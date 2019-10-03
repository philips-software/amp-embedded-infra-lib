#include "services/network/HttpPageWebSocket.hpp"
#include "services/network/HttpErrors.hpp"

namespace services
{
    HttpPageWebSocket::HttpPageWebSocket(infra::BoundedConstString path, WebSocketObserverFactory& webSocketObserverFactory, Address address)
        : path(path)
        , webSocketObserverFactory(webSocketObserverFactory)
        , address(address.address)
    {}

    bool HttpPageWebSocket::ServesRequest(const infra::Tokenizer& pathTokens) const
    {
        return pathTokens.TokenAndRest(0) == path;
    }
    
    void HttpPageWebSocket::RespondToRequest(services::HttpRequestParser& parser, services::HttpServerConnection& connection)
    {
        if (parser.Header("Upgrade") == "websocket" && parser.Header("Connection") == "Upgrade" && !parser.Header("Sec-WebSocket-Key").empty())
        {
            connection.TakeOverConnection(*this);

            webSocketObserverFactory.CreateWebSocketObserver(Subject(), parser.Header("Sec-WebSocket-Key"), address);
        }
        else
            connection.SendResponse(services::HttpResponseNotFound::Instance());
    }

    void HttpPageWebSocket::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& stream)
    {
        std::abort();
    }

    void HttpPageWebSocket::DataReceived()
    {}

    void HttpPageWebSocket::ClosingConnection()
    {
        webSocketObserverFactory.CancelCreation();
    }
}
