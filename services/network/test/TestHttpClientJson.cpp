#include "infra/stream/test/StreamMock.hpp"
#include "infra/syntax/test_doubles/JsonStreamingParserMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/BoundedStringMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/HttpClientJson.hpp"
#include "services/network/test_doubles/HttpClientMock.hpp"
#include "gmock/gmock.h"

class HttpClientJsonMock
    : public services::HttpClientJson
{
public:
    using services::HttpClientJson::HttpClientJson;

    MOCK_CONST_METHOD0(Headers, services::HttpHeaders());
    MOCK_METHOD0(TopJsonObjectVisitor, infra::JsonObjectVisitor&());
    MOCK_METHOD1(Error, void(bool intermittentFailure));
    MOCK_METHOD0(Done, void());

    using services::HttpClientJson::ContentError;
    using services::HttpClientJson::Detaching;
};

class HttpClientJsonTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    HttpClientJsonTest()
    {
        EXPECT_EQ("hostname", httpClientObserverFactory->Hostname());
        EXPECT_EQ(443, httpClientObserverFactory->Port());
    }

    testing::StrictMock<services::HttpClientConnectorMock> httpClientConnector;
    services::HttpClientObserverFactory* httpClientObserverFactory = nullptr;
    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(httpClientConnector, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&httpClientObserverFactory));
        } };
    infra::BoundedString::WithStorage<64> url{ "https://hostname/path" };
    services::HttpClientJson::JsonParserCreator<32, 64, 5> jsonParserCreator;
    testing::StrictMock<HttpClientJsonMock> controller{ url, services::HttpClientJson::ConnectionInfo{ jsonParserCreator, 443, httpClientConnector } };
    testing::StrictMock<infra::MockCallback<void()>> onStopped;
    testing::StrictMock<services::HttpClientMock> httpClient;

    testing::StrictMock<infra::JsonObjectVisitorMock> jsonObjectVisitor;
    std::array<services::HttpHeader, 2> headersIn{ { { "api-version", "1" }, { "authorization", "Bearer xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx" } } };
};

TEST_F(HttpClientJsonTest, should_do_Get_request_with_correct_headers_when_Connected)
{
    services::HttpHeaders headers;
    EXPECT_CALL(httpClient, Get("/path", testing::_)).WillOnce(testing::SaveArg<1>(&headers));
    EXPECT_CALL(controller, Headers()).WillOnce(testing::Return(headersIn));
    EXPECT_CALL(controller, TopJsonObjectVisitor()).WillOnce(testing::ReturnRef(jsonObjectVisitor));
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client)
        { httpClient.Attach(client); });

    HttpHeadersEquals({ { "api-version", "1" }, { "authorization", "Bearer xxxxx-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx" } }, headers);
    EXPECT_CALL(controller, Error(true));
}

TEST_F(HttpClientJsonTest, unsuccessful_status_is_reported)
{
    EXPECT_CALL(httpClient, Get("/path", testing::_));
    EXPECT_CALL(controller, Headers()).WillOnce(testing::Return(headersIn));
    EXPECT_CALL(controller, TopJsonObjectVisitor()).WillOnce(testing::ReturnRef(jsonObjectVisitor));
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client)
        { httpClient.Attach(client); });

    EXPECT_CALL(controller, Error(false));
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().StatusAvailable(services::HttpStatusCode::BadRequest);
}

TEST_F(HttpClientJsonTest, non_JSON_content_is_rejected)
{
    EXPECT_CALL(httpClient, Get("/path", testing::_));
    EXPECT_CALL(controller, Headers()).WillOnce(testing::Return(headersIn));
    EXPECT_CALL(controller, TopJsonObjectVisitor()).WillOnce(testing::ReturnRef(jsonObjectVisitor));
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client)
        { httpClient.Attach(client); });

    EXPECT_CALL(controller, Error(false));
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().HeaderAvailable(services::HttpHeader{ "content-type", "application/text" });
}

TEST_F(HttpClientJsonTest, feed_json_data)
{
    EXPECT_CALL(httpClient, Get("/path", testing::_));
    EXPECT_CALL(controller, Headers()).WillOnce(testing::Return(headersIn));
    EXPECT_CALL(controller, TopJsonObjectVisitor()).WillOnce(testing::ReturnRef(jsonObjectVisitor));
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client)
        { httpClient.Attach(client); });

    testing::StrictMock<infra::StreamReaderMock> reader;
    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(false)).WillOnce(testing::Return(true));
    EXPECT_CALL(reader, ExtractContiguousRange(testing::_)).WillOnce(testing::Return(infra::MakeStringByteRange(R"({ "entry": "value" })")));
    EXPECT_CALL(jsonObjectVisitor, VisitString("entry", "value"));
    EXPECT_CALL(jsonObjectVisitor, Close());
    httpClient.Observer().BodyAvailable(infra::UnOwnedSharedPtr(reader));
    EXPECT_CALL(controller, Done());
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().BodyComplete();
}

TEST_F(HttpClientJsonTest, ParseError_reports_Error)
{
    EXPECT_CALL(httpClient, Get("/path", testing::_));
    EXPECT_CALL(controller, Headers()).WillOnce(testing::Return(headersIn));
    EXPECT_CALL(controller, TopJsonObjectVisitor()).WillOnce(testing::ReturnRef(jsonObjectVisitor));
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client)
        { httpClient.Attach(client); });

    testing::StrictMock<infra::StreamReaderMock> reader;
    auto readerPtr(infra::UnOwnedSharedPtr(reader));
    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(false));
    EXPECT_CALL(reader, ExtractContiguousRange(testing::_)).WillOnce(testing::Return(infra::MakeStringByteRange(R"({ ] })")));
    EXPECT_CALL(jsonObjectVisitor, ParseError()).WillOnce(testing::Invoke([this]()
        { controller.ContentError(); }));
    EXPECT_CALL(httpClient, CloseConnection()).WillOnce(testing::Invoke([this, &readerPtr]()
        {
        controller.Detaching();
        EXPECT_TRUE(readerPtr == nullptr); }));
    EXPECT_CALL(controller, Error(false));
    httpClient.Observer().BodyAvailable(std::move(readerPtr));
}

TEST_F(HttpClientJsonTest, ContentError_during_parsing_closes_connection)
{
    EXPECT_CALL(httpClient, Get("/path", testing::_));
    EXPECT_CALL(controller, Headers()).WillOnce(testing::Return(headersIn));
    EXPECT_CALL(controller, TopJsonObjectVisitor()).WillOnce(testing::ReturnRef(jsonObjectVisitor));
    httpClientObserverFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client)
        { httpClient.Attach(client); });

    testing::StrictMock<infra::StreamReaderMock> reader;
    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(false)).WillOnce(testing::Return(true));
    EXPECT_CALL(reader, ExtractContiguousRange(testing::_)).WillOnce(testing::Return(infra::MakeStringByteRange(R"({ "entry": "value")")));
    EXPECT_CALL(jsonObjectVisitor, VisitString("entry", "value")).WillOnce(testing::Invoke([this](infra::BoundedConstString, infra::BoundedConstString)
        { controller.ContentError(); }));
    EXPECT_CALL(controller, Error(false));
    EXPECT_CALL(httpClient, CloseConnection());
    httpClient.Observer().BodyAvailable(infra::UnOwnedSharedPtr(reader));
}
