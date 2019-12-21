#ifndef NETWORK_HTTP_MOCK_HPP
#define NETWORK_HTTP_MOCK_HPP

#include "gmock/gmock.h"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "services/network/HttpClient.hpp"

namespace services
{
    void HttpHeadersEquals(const std::vector<services::HttpHeader>& expected, const services::HttpHeaders& actual);
    void PrintTo(const HttpHeader& header, std::ostream* stream);

    class HttpClientObserverMock
        : public HttpClientObserver
    {
    public:
        MOCK_METHOD0(Connected, void());
        MOCK_METHOD0(Detaching, void());

        MOCK_METHOD1(StatusAvailable, void(HttpStatusCode statusCode));
        MOCK_METHOD1(HeaderAvailable, void(HttpHeader header));
        MOCK_METHOD1(BodyAvailable, void(infra::SharedPtr<infra::StreamReader>&& reader));
        MOCK_METHOD0(BodyComplete, void());
        MOCK_METHOD1(SendStreamAvailable, void(infra::SharedPtr<infra::StreamWriter>&& writer));
        MOCK_CONST_METHOD1(FillContent, void(infra::StreamWriter& writer));
    };

    class HttpClientObserverFactoryMock
        : public HttpClientObserverFactory
    {
    public:
        MOCK_CONST_METHOD0(Hostname, infra::BoundedConstString());
        MOCK_CONST_METHOD0(Port, uint16_t());
        MOCK_METHOD1(ConnectionEstablished, void(infra::AutoResetFunction<void(infra::SharedPtr<HttpClientObserver> client)>&& createdClientObserver));
        MOCK_METHOD1(ConnectionFailed, void(ConnectFailReason reason));
    };

    class HttpClientMock
        : public HttpClient
    {
    public:
        MOCK_METHOD2(Get, void(infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD2(Head, void(infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD2(Connect, void(infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD2(Options, void(infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD3(Post, void(infra::BoundedConstString, infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD3(Post, void(infra::BoundedConstString, std::size_t, HttpHeaders));
        MOCK_METHOD2(Post, void(infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD3(Put, void(infra::BoundedConstString, infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD3(Put, void(infra::BoundedConstString, std::size_t, HttpHeaders));
        MOCK_METHOD2(Put, void(infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD3(Patch, void(infra::BoundedConstString, infra::BoundedConstString, HttpHeaders));
        MOCK_METHOD3(Delete, void(infra::BoundedConstString, infra::BoundedConstString, HttpHeaders));
        
        MOCK_METHOD0(AckReceived, void());
        MOCK_METHOD0(Close, void());

        MOCK_METHOD0(GetConnection, Connection&());
    };

    class HttpClientConnectorMock
        : public HttpClientConnector
    {
    public:
        MOCK_METHOD1(Connect, void(HttpClientObserverFactory& factory));
        MOCK_METHOD1(CancelConnect, void(HttpClientObserverFactory& factory));
    };
}

#endif
