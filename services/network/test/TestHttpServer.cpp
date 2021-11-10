#include "gmock/gmock.h"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/HttpServer.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "services/network/test_doubles/HttpServerMock.hpp"

class HttpServerTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    HttpServerTest()
        : connectionPtr(infra::UnOwnedSharedPtr(connection))
        , execute([this]() { EXPECT_CALL(connectionFactoryMock, Listen(80, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr))); })
        , httpServer(connectionFactoryMock, 80)
    {}

    ~HttpServerTest()
    {
        httpServer.Stop(infra::emptyFunction);
    }

    void DontExpectPageServerRequest(const std::string& request)
    {
        infra::ConstByteRange data = infra::MakeStringByteRange(request);
        connection.SimulateDataReceived(data);
        ExecuteAllActions();
    }

    void SendResponse(infra::BoundedConstString status, infra::BoundedConstString contentType, infra::BoundedConstString body)
    {
        testing::StrictMock<services::HttpResponseMock> response(1024);
        EXPECT_CALL(response, ContentType()).WillOnce(testing::Return(contentType));
        EXPECT_CALL(response, AddHeaders(testing::_));
        EXPECT_CALL(response, WriteBody(testing::_)).WillOnce(testing::Invoke([body](infra::TextOutputStream& stream) { stream << body; }));
        EXPECT_CALL(response, Status()).WillOnce(testing::Return(status));
        httpConnection->SendResponse(response);
    }

    void CheckHttpResponse(const char* result, const char* body)
    {
        std::string response = std::string("HTTP/1.1 ") + result + "\r\n" +
            "Content-Length: " + std::to_string(std::strlen(body)) + "\r\n" +
            "Content-Type: application/json\r\n" +
            "\r\n" +
            body;

        EXPECT_EQ(std::vector<uint8_t>(response.begin(), response.end()), connection.sentData);
        connection.sentData.clear();
    }

    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::ConnectionStub> connectionPtr;
    testing::StrictMock<services::ConnectionFactoryMock> connectionFactoryMock;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    infra::Execute execute;
    services::DefaultHttpServer::WithBuffer<256> httpServer;
    services::HttpServerConnection* httpConnection;

    infra::StringInputStreamReader emptyReader{ "" };
    infra::SharedPtr<infra::StreamReaderWithRewinding> emptyReaderPtr { infra::UnOwnedSharedPtr(emptyReader) };
};

class HttpServerWithSimplePageTest
    : public HttpServerTest
{
public:
    HttpServerWithSimplePageTest()
    {
        httpServer.AddPage(httpPage);
    }

    void ExpectPageServerRequest(services::HttpVerb verb, const std::string& request)
    {
        EXPECT_CALL(httpPage, ServesRequest(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(httpPage, RespondToRequest(testing::_, testing::_)).WillOnce(testing::Invoke([this, verb](services::HttpRequestParser& parser, services::HttpServerConnection& connection) {
            EXPECT_EQ(verb, parser.Verb());
            httpConnection = &connection;
        }));
        infra::ConstByteRange data = infra::MakeStringByteRange(request);
        connection.SimulateDataReceived(data);
        ExecuteAllActions();
    }

    void ExpectPageServerRequestWithBody(services::HttpVerb verb, const std::string& request, const std::string& body)
    {
        EXPECT_CALL(httpPage, ServesRequest(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(httpPage, RespondToRequest(testing::_, testing::_)).WillOnce(testing::Invoke([this, verb, body](services::HttpRequestParser& parser, services::HttpServerConnection& connection) {
            EXPECT_EQ(verb, parser.Verb());
            EXPECT_EQ(body, parser.BodyBuffer());
            httpConnection = &connection;
        }));
        infra::ConstByteRange data = infra::MakeStringByteRange(request);
        connection.SimulateDataReceived(data);
        ExecuteAllActions();
    }

    void ExpectPageServerRequestWithHeader(services::HttpVerb verb, const std::string& request, const services::HttpHeader& header)
    {
        EXPECT_CALL(httpPage, ServesRequest(testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(httpPage, RespondToRequest(testing::_, testing::_)).WillOnce(testing::Invoke([this, verb, header](services::HttpRequestParser& parser, services::HttpServerConnection& connection) {
            EXPECT_EQ(verb, parser.Verb());
            EXPECT_EQ(header.Value(), parser.Header(header.Field()));
            httpConnection = &connection;
        }));
        infra::ConstByteRange data = infra::MakeStringByteRange(request);
        connection.SimulateDataReceived(data);
        ExecuteAllActions();
    }

public:
    testing::StrictMock<services::SimpleHttpPageMock> httpPage;
};

class HttpServerErrorTest
    : public HttpServerWithSimplePageTest
    , public testing::WithParamInterface<const char*>
{};

TEST_F(HttpServerWithSimplePageTest, accept_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, accept_ipv6_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv6AddressLocalHost());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, wrong_start_url_results_in_error)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("PUT /path HTTP/1.1 \r\n\r\n {}");

    EXPECT_CALL(httpPage, ServesRequest(testing::_)).WillOnce(testing::Return(false));
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    EXPECT_CALL(connection, AbortAndDestroyMock());
    connection.AbortAndDestroy();
    CheckHttpResponse("404 Not Found", "{}");
}

TEST_F(HttpServerWithSimplePageTest, too_long_http_request_results_in_error)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("PUT /path/subpath/subpath/subpath/subpath/subpath/subpath/subpath HTTP/1.1\r\nContent-Type: application/json\r\nHost: 192.168.0.98\r\nConnection: Close\r\nContent-Length: 688\r\n\r\n { \"data\":\"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\" }");

    EXPECT_CALL(httpPage, ServesRequest(testing::_)).WillOnce(testing::Return(true));
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    CheckHttpResponse("500 Internal Server Error", R"({ "error": "Out of memory" })");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, unsupported_verb_results_in_error)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::ConstByteRange data = infra::MakeStringByteRange("UPDATE /path HTTP/1.1 \r\n\r\n {}");

    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    CheckHttpResponse("400 Bad request", "{}");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, request_results_in_get)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    ExpectPageServerRequest(services::HttpVerb::get, "GET /path HTTP/1.1 \r\n\r\n");
    EXPECT_CALL(connection, AbortAndDestroyMock());
    connection.AbortAndDestroy();
}

TEST_F(HttpServerWithSimplePageTest, request_without_content_length_still_results_in_put)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    ExpectPageServerRequestWithBody(services::HttpVerb::put, "PUT /path HTTP/1.1 \r\n\r\ndatadata", "datadata");
    EXPECT_CALL(connection, AbortAndDestroyMock());
    connection.AbortAndDestroy();
}

TEST_F(HttpServerWithSimplePageTest, request_parses_header)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    ExpectPageServerRequestWithHeader(services::HttpVerb::get, "GET /path HTTP/1.1 \r\nAccept-Encoding: identity\r\n\r\n", { "accept-encoding", "identity" });
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, unfinished_request_is_not_served)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    EXPECT_CALL(httpPage, ServesRequest(testing::_)).WillOnce(testing::Return(true));
    DontExpectPageServerRequest("GET /path HTTP/1.1 \r\nContent-Length: 1\r\n\r\n");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, finished_request_is_served)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    ExpectPageServerRequest(services::HttpVerb::get, "GET /path HTTP/1.1 \r\nContent-Length: 1\r\n\r\nA");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, content_length_header_is_case_insensitive)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    ExpectPageServerRequest(services::HttpVerb::get, "GET /path HTTP/1.1 \r\ncontent-length: 1\r\n\r\nA");
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, double_request_results_in_closed_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    ExpectPageServerRequest(services::HttpVerb::get, "GET /path HTTP/1.1 \r\n\r\n");

    EXPECT_CALL(connection, AbortAndDestroyMock());
    infra::ConstByteRange data = infra::MakeStringByteRange("GET /path HTTP/1.1 \r\n\r\n");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();
    testing::Mock::VerifyAndClearExpectations(&connection);
}

TEST_F(HttpServerWithSimplePageTest, split_message_is_accepted)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    std::string rest = R"({"other":"param"})";

    ExpectPageServerRequest(services::HttpVerb::get, std::string("GET /path") +
        " HTTP/1.1\r\nAccept-Encoding: identity\r\nHost: 192.168.1.56\r\nContent-Length: " + std::to_string(rest.size()) + "\r\nContent-Type: application-json\r\n\r\n");

    connection.SimulateDataReceived(infra::MakeStringByteRange(rest));
    ExecuteAllActions();
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, send_100_response_when_expect_100_in_header)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    ExpectPageServerRequest(services::HttpVerb::get, "GET /path HTTP/1.1 \r\nExpect: 100-continue\r\n\r\n");
    SendResponse("", "", "");

    ExecuteAllActions();

    std::string expectedResponseFirstPart = std::string("HTTP/1.1 ") + "100 Continue" + "\r\n" +
        "Content-Length: 0" + "\r\n" +
        "Content-Type: application/json\r\n" +
        "Strict-Transport-Security: max-age=31536000\r\n\r\n";

    std::vector<uint8_t> responseFirstPart = connection.sentData;
    responseFirstPart.erase(responseFirstPart.begin() + expectedResponseFirstPart.size(), responseFirstPart.end());
    connection.sentData.erase(connection.sentData.begin(), connection.sentData.begin() + expectedResponseFirstPart.size());
    EXPECT_EQ(std::vector<uint8_t>(expectedResponseFirstPart.begin(), expectedResponseFirstPart.end()), responseFirstPart);

    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, split_response_when_not_enough_available_in_stream)
{
    testing::StrictMock<services::ConnectionMock> connection;
    EXPECT_CALL(connection, MaxSendStreamSize()).WillRepeatedly(testing::Return(555));
    EXPECT_CALL(connection, RequestSendStream(555)).Times(testing::AnyNumber());
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::BoundedConstString request = "GET /path HTTP/1.1 \r\n\r\n";
    infra::StringInputStreamReader reader(request);
    auto readerPtr = infra::UnOwnedSharedPtr(reader);
    EXPECT_CALL(connection, ReceiveStream()).WillOnce(testing::Return(readerPtr));
    EXPECT_CALL(httpPage, ServesRequest(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(connection, AckReceived()).Times(2);
    EXPECT_CALL(httpPage, RespondToRequest(testing::_, testing::_)).WillOnce(testing::Invoke([this](services::HttpRequestParser& parser, services::HttpServerConnection& connection)
    {
        httpConnection = &connection;
        SendResponse("200 OK", "application/text", "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
    }));

    infra::StringOutputStream::WithStorage<80> stream1;
    connection.Observer().SendStreamAvailable(infra::UnOwnedSharedPtr(stream1.Writer()));
    EXPECT_EQ("HTTP/1.1 200 OK\r\nContent-Length: 100\r\nContent-Type: application/text\r\n\r\n01234567", stream1.Storage());

    infra::StringOutputStream::WithStorage<80> stream2;
    connection.Observer().SendStreamAvailable(infra::UnOwnedSharedPtr(stream2.Writer()));
    EXPECT_EQ("89012345678901234567890123456789012345678901234567890123456789012345678901234567", stream2.Storage());
}

TEST_F(HttpServerWithSimplePageTest, second_connection_forces_idle_connection_to_close)
{
    testing::StrictMock<services::ConnectionMock> connectionFirst;
    EXPECT_CALL(connectionFirst, MaxSendStreamSize()).WillOnce(testing::Return(555));
    EXPECT_CALL(connectionFirst, RequestSendStream(555));
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connectionFirst, services::IPv4AddressLocalHost());

    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    EXPECT_CALL(connectionFirst, CloseAndDestroy());
    ForwardTime(std::chrono::seconds(10));
    testing::Mock::VerifyAndClearExpectations(&connectionFirst);

    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerWithSimplePageTest, non_idle_connection_is_not_closed_by_second_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    ExpectPageServerRequest(services::HttpVerb::get, "GET /path HTTP/1.1 \r\nExpect: 100-continue\r\n\r\n");

    testing::StrictMock<services::ConnectionStub> connectionSecond;
    infra::SharedPtr<services::ConnectionStub> connectionSecondPtr(infra::UnOwnedSharedPtr(connectionSecond));
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connectionSecond, services::IPv4AddressLocalHost());

    EXPECT_CALL(connection, CloseAndDestroyMock());
    SendResponse("", "", "");
}

TEST_F(HttpServerWithSimplePageTest, choose_correct_url)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    testing::StrictMock<services::HttpPageMock> secondHttpPage;
    httpServer.AddPage(secondHttpPage);

    EXPECT_CALL(secondHttpPage, ServesRequest(testing::_)).WillOnce(testing::Return(false));
    ExpectPageServerRequest(services::HttpVerb::get, "GET /path HTTP/1.1 \r\nExpect: 100-continue\r\n\r\n");

    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(HttpServerTest, connection_is_kept_open_by_page)
{
    testing::StrictMock<services::HttpPageMock> page;
    httpServer.AddPage(page);

    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());

    infra::WeakPtr<void> observer = connection.ObserverPtr();

    EXPECT_CALL(page, ServesRequest(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(page, RequestReceived(testing::_, testing::_)).WillOnce(testing::Invoke([this](services::HttpRequestParser& parser, services::HttpServerConnection& connection) {
        httpConnection = &connection;
    }));
    infra::SharedPtr<infra::StreamReaderWithRewinding> savedReader;
    EXPECT_CALL(page, DataReceived(testing::_)).WillOnce(testing::Invoke([&savedReader](const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader) { savedReader = reader; }));
    EXPECT_CALL(page, Close());
    infra::ConstByteRange data = infra::MakeStringByteRange("GET /path HTTP/1.1 \r\n\r\n");
    connection.SimulateDataReceived(data);
    ExecuteAllActions();

    EXPECT_CALL(connection, AbortAndDestroyMock());
    connection.AbortAndDestroy();

    EXPECT_NE(nullptr, observer.lock());
    savedReader = nullptr;
    EXPECT_EQ(nullptr, observer.lock());
}
