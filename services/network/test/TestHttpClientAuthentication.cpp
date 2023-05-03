#include "infra/stream/test/StreamMock.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/SharedOptional.hpp"
#include "services/network/HttpClientAuthentication.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/HttpClientMock.hpp"
#include "gmock/gmock.h"

namespace
{
    class HttpClientAuthentication
        : public services::HttpClientAuthentication
    {
    public:
        template<std::size_t MaxHeaders>
        using WithMaxHeaders = infra::WithStorage<HttpClientAuthentication, infra::BoundedVector<services::HttpHeader>::WithMaxSize<MaxHeaders>>;

        using services::HttpClientAuthentication::HttpClientAuthentication;

        MOCK_METHOD2(Authenticate, void(infra::BoundedConstString scheme, infra::BoundedConstString value));
        MOCK_CONST_METHOD0(AuthenticationHeader, infra::BoundedConstString());
        MOCK_CONST_METHOD0(Retry, bool());
        MOCK_METHOD0(Reset, void());
    };
}

class HttpClientAuthenticationTest
    : public testing::Test
{
public:
    HttpClientAuthenticationTest()
    {
        httpClient.Attach(infra::MakeContainedSharedObject(clientAuthentication, httpClientObserver.Emplace()));
        EXPECT_CALL(*httpClientObserver, Attached());
        clientAuthentication.Attach(httpClientObserver.MakePtr());
    }

    ~HttpClientAuthenticationTest()
    {
        EXPECT_CALL(*httpClientObserver, Detaching());
    }

    void CheckHeaders(services::HttpHeaders headersToCheck, infra::BoundedConstString headerValueToAdd)
    {
        decltype(headers) copy(headers);
        copy.push_back({ "Authorization", headerValueToAdd });
        EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(copy), headersToCheck));
    }

    infra::SharedOptional<testing::StrictMock<services::HttpClientObserverMock>> httpClientObserver;
    testing::StrictMock<HttpClientAuthentication::WithMaxHeaders<4>> clientAuthentication;
    testing::StrictMock<services::HttpClientMock> httpClient;

    infra::BoundedVector<const services::HttpHeader>::WithMaxSize<3> headers{ { services::HttpHeader("name", "value") } };
};

TEST_F(HttpClientAuthenticationTest, Get_request_is_forwarded)
{
    EXPECT_CALL(httpClient, Get("target", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Get("target", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, Head_request_is_forwarded)
{
    EXPECT_CALL(httpClient, Head("target", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Head("target", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, Connect_request_is_forwarded)
{
    EXPECT_CALL(httpClient, Connect("target", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Connect("target", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, Options_request_is_forwarded)
{
    EXPECT_CALL(httpClient, Options("target", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Options("target", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, Post_request_is_forwarded_1)
{
    EXPECT_CALL(httpClient, Post("target", "contents", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, infra::BoundedConstString contents, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Post("target", "contents", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, Post_request_is_forwarded_3)
{
    EXPECT_CALL(httpClient, Post("target", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Post("target", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, Put_request_is_forwarded_1)
{
    EXPECT_CALL(httpClient, Put("target", "contents", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, infra::BoundedConstString contents, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Put("target", "contents", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, Put_request_is_forwarded_3)
{
    EXPECT_CALL(httpClient, Put("target", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Put("target", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, Delete_request_is_forwarded)
{
    EXPECT_CALL(httpClient, Delete("target", "contents", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, infra::BoundedConstString contents, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Delete("target", "contents", infra::MakeRange(headers));
}

TEST_F(HttpClientAuthenticationTest, AckReceived_is_forwarded)
{
    EXPECT_CALL(httpClient, AckReceived());
    httpClientObserver->Subject().AckReceived();
}

TEST_F(HttpClientAuthenticationTest, Close_is_forwarded)
{
    EXPECT_CALL(httpClient, CloseConnection());
    httpClientObserver->Subject().CloseConnection();
}

TEST_F(HttpClientAuthenticationTest, GetConnection_is_forwarded)
{
    testing::StrictMock<services::ConnectionMock> connection;
    EXPECT_CALL(httpClient, GetConnection()).WillOnce(testing::ReturnRef(connection));
    EXPECT_EQ(&connection, &httpClientObserver->Subject().GetConnection());
}

TEST_F(HttpClientAuthenticationTest, normal_flow_is_forwarded)
{
    testing::StrictMock<infra::StreamWriterMock> writer;
    EXPECT_CALL(*httpClientObserver, FillContent(testing::Ref(writer)));
    httpClient.Observer().FillContent(writer);

    EXPECT_CALL(*httpClientObserver, SendStreamAvailable(testing::_));
    httpClient.Observer().SendStreamAvailable(infra::UnOwnedSharedPtr(writer));

    EXPECT_CALL(*httpClientObserver, StatusAvailable(services::HttpStatusCode::OK));
    httpClient.Observer().StatusAvailable(services::HttpStatusCode::OK);

    EXPECT_CALL(*httpClientObserver, HeaderAvailable(services::HttpHeader{ "name", "value" }));
    httpClient.Observer().HeaderAvailable({ "name", "value" });
    EXPECT_CALL(*httpClientObserver, HeaderAvailable(services::HttpHeader{ "WWW-Authenticate", "value" }));
    httpClient.Observer().HeaderAvailable({ "WWW-Authenticate", "value" });

    testing::StrictMock<infra::StreamReaderMock> reader;
    EXPECT_CALL(*httpClientObserver, BodyAvailable(testing::_));
    httpClient.Observer().BodyAvailable(infra::UnOwnedSharedPtr(reader));

    EXPECT_CALL(*httpClientObserver, BodyComplete());
    httpClient.Observer().BodyComplete();
}

TEST_F(HttpClientAuthenticationTest, unauthorized_is_not_retried)
{
    httpClient.Observer().StatusAvailable(services::HttpStatusCode::Unauthorized);

    httpClient.Observer().HeaderAvailable({ "name", "value" });
    EXPECT_CALL(clientAuthentication, Authenticate("scheme", "value"));
    httpClient.Observer().HeaderAvailable({ "WWW-Authenticate", "scheme value" });

    testing::StrictMock<infra::StreamReaderMock> reader;
    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(true));
    httpClient.Observer().BodyAvailable(infra::UnOwnedSharedPtr(reader));

    EXPECT_CALL(clientAuthentication, Retry()).WillOnce(testing::Return(false));
    EXPECT_CALL(*httpClientObserver, StatusAvailable(services::HttpStatusCode::Unauthorized));
    EXPECT_CALL(*httpClientObserver, BodyComplete());
    httpClient.Observer().BodyComplete();
}

TEST_F(HttpClientAuthenticationTest, unauthorized_is_retried)
{
    EXPECT_CALL(httpClient, Get("target", testing::_));
    EXPECT_CALL(clientAuthentication, AuthenticationHeader()).WillOnce(testing::Return("header contents"));
    httpClientObserver->Subject().Get("target", infra::MakeRange(headers));

    httpClient.Observer().StatusAvailable(services::HttpStatusCode::Unauthorized);

    httpClient.Observer().HeaderAvailable({ "name", "value" });
    EXPECT_CALL(clientAuthentication, Authenticate("scheme", "value"));
    httpClient.Observer().HeaderAvailable({ "WWW-Authenticate", "scheme value" });

    testing::StrictMock<infra::StreamReaderMock> reader;
    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(true));
    httpClient.Observer().BodyAvailable(infra::UnOwnedSharedPtr(reader));

    EXPECT_CALL(clientAuthentication, Retry()).WillOnce(testing::Return(true));
    EXPECT_CALL(clientAuthentication, Reset());
    EXPECT_CALL(httpClient, Get("target", testing::_)).WillOnce(testing::Invoke([this](infra::BoundedConstString requestTarget, services::HttpHeaders headers)
        { CheckHeaders(headers, "header contents"); }));
    httpClient.Observer().BodyComplete();
}
