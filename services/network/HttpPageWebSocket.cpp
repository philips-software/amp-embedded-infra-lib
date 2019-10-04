#include "mbedtls/sha1.h"
#include "services/network/HttpPageWebSocket.hpp"
#include "services/network/HttpErrors.hpp"

namespace services
{
    HttpPageWebSocket::HttpPageWebSocket(infra::BoundedConstString path, WebSocketObserverFactory& webSocketObserverFactory)
        : services::HttpResponse(0)
        , path(path)
        , webSocketObserverFactory(webSocketObserverFactory)
    {}

    bool HttpPageWebSocket::ServesRequest(const infra::Tokenizer& pathTokens) const
    {
        return pathTokens.TokenAndRest(0) == path;
    }
    
    void HttpPageWebSocket::RespondToRequest(services::HttpRequestParser& parser, services::HttpServerConnection& connection)
    {
        if (parser.Header("Upgrade") == "websocket" && parser.Header("Connection") == "Upgrade" && !parser.Header("Sec-WebSocket-Key").empty())
        {
            webSocketKey = parser.Header("Sec-WebSocket-Key").substr(0, webSocketKey.max_size());
            static const infra::BoundedConstString webSocketGuid("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
            webSocketKey.append(webSocketGuid);

            connection.SendResponseWithoutNextRequest(*this);
            connection.TakeOverConnection(*this);

            webSocketObserverFactory.CreateWebSocketObserver(Subject());
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

    infra::BoundedConstString HttpPageWebSocket::Status() const
    {
        return "101 Switching Protocols";
    }

    void HttpPageWebSocket::WriteBody(infra::TextOutputStream& stream) const
    {}

    void HttpPageWebSocket::AddHeaders(HttpResponseHeaderBuilder& builder) const
    {
        std::array<uint8_t, 20> sha1Digest;
        mbedtls_sha1(reinterpret_cast<const uint8_t*>(webSocketKey.begin()), webSocketKey.size(), sha1Digest.data());    //TICS !INT#030

        builder.AddHeader("Upgrade", "websocket");
        builder.AddHeader("Connection", "Upgrade");
        builder.AddHeader("Sec-WebSocket-Accept");
        builder.Stream() << infra::AsBase64(sha1Digest);
    }
}
