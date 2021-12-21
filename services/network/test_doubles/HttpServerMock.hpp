#ifndef NETWORK_HTTP_SERVER_MOCK_HPP
#define NETWORK_HTTP_SERVER_MOCK_HPP

#include "gmock/gmock.h"
#include "services/network/HttpServer.hpp"

namespace services
{
    class HttpServerConnectionMock
        : public HttpServerConnection
    {
    public:
        MOCK_METHOD1(SendResponse, void(const HttpResponse& response));
        MOCK_METHOD1(SendResponseWithoutNextRequest, void(const HttpResponse& response));
        MOCK_METHOD1(TakeOverConnection, void(ConnectionObserver& observer));
    };

    class HttpPageMock
        : public HttpPage
    {
    public:
        MOCK_CONST_METHOD1(ServesRequest, bool(const infra::Tokenizer& pathTokens));
        MOCK_METHOD2(RequestReceived, void(services::HttpRequestParser& parser, services::HttpServerConnection& connection));
        MOCK_METHOD1(DataReceived, void(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader));
        MOCK_METHOD0(Close, void());
    };

    class SimpleHttpPageMock
        : public SimpleHttpPage
    {
    public:
        MOCK_CONST_METHOD1(ServesRequest, bool(const infra::Tokenizer& pathTokens));
        MOCK_METHOD2(RespondToRequest, void(HttpRequestParser& parser, HttpServerConnection& connection));
    };

    class HttpResponseMock
        : public HttpResponse
    {
    public:
        HttpResponseMock(std::size_t maxBodySize)
            : HttpResponse(maxBodySize)
        {}

        MOCK_CONST_METHOD0(Status, infra::BoundedConstString());
        MOCK_CONST_METHOD1(WriteBody, void(infra::TextOutputStream& stream));
        MOCK_CONST_METHOD0(ContentType, infra::BoundedConstString());
        MOCK_CONST_METHOD1(AddHeaders, void(services::HttpResponseHeaderBuilder& builder));
    };
}

#endif
