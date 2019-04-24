#include "gmock/gmock.h"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/StdStringOutputStream.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/HttpClientImpl.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "services/network/test_doubles/HttpClientMock.hpp"

TEST(HttpTest, parse_components_from_url)
{
    EXPECT_EQ("http", services::SchemeFromUrl("http://host/path"));
    EXPECT_EQ("host", services::HostFromUrl("http://host/path"));
    EXPECT_EQ("/path", services::PathFromUrl("http://host/path"));
    EXPECT_EQ("/path/more", services::PathFromUrl("http://host/path/more"));
}

TEST(HttpTest, write_formatted_HttpHeader_to_stream)
{
    services::HttpHeader header{ "Key", "Value" };
    infra::StdStringOutputStream::WithStorage stream;

    stream << header;

    EXPECT_EQ("Key:Value", stream.Storage());
}

struct HttpStatusCodeWithString
{
    services::HttpStatusCode code;
    std::string string;
};

class HttpStatusMessageFormattingTest
    : public testing::TestWithParam<HttpStatusCodeWithString>
{
public:
    infra::StdStringOutputStream::WithStorage stream;
};

TEST_P(HttpStatusMessageFormattingTest, write_formatted_HttpStatusCode_to_stream)
{
    auto parameter = GetParam();
    stream << parameter.code;
    EXPECT_EQ(parameter.string, stream.Storage());
}

INSTANTIATE_TEST_CASE_P(HttpStatusMessageTest, HttpStatusMessageFormattingTest,
    testing::Values(
        HttpStatusCodeWithString{ services::HttpStatusCode::Continue, "Continue" },
        HttpStatusCodeWithString{ services::HttpStatusCode::SwitchingProtocols, "SwitchingProtocols" },
        HttpStatusCodeWithString{ services::HttpStatusCode::OK, "OK" },
        HttpStatusCodeWithString{ services::HttpStatusCode::Created, "Created" },
        HttpStatusCodeWithString{ services::HttpStatusCode::Accepted, "Accepted" },
        HttpStatusCodeWithString{ services::HttpStatusCode::NonAuthorativeInformation, "NonAuthorativeInformation" },
        HttpStatusCodeWithString{ services::HttpStatusCode::NoContent, "NoContent" },
        HttpStatusCodeWithString{ services::HttpStatusCode::ResetContent, "ResetContent" },
        HttpStatusCodeWithString{ services::HttpStatusCode::PartialContent, "PartialContent" },
        HttpStatusCodeWithString{ services::HttpStatusCode::MultipleChoices, "MultipleChoices" },
        HttpStatusCodeWithString{ services::HttpStatusCode::MovedPermanently, "MovedPermanently" },
        HttpStatusCodeWithString{ services::HttpStatusCode::Found, "Found" },
        HttpStatusCodeWithString{ services::HttpStatusCode::SeeOther, "SeeOther" },
        HttpStatusCodeWithString{ services::HttpStatusCode::NotModified, "NotModified" },
        HttpStatusCodeWithString{ services::HttpStatusCode::UseProxy, "UseProxy" },
        HttpStatusCodeWithString{ services::HttpStatusCode::TemporaryRedirect, "TemporaryRedirect" },
        HttpStatusCodeWithString{ services::HttpStatusCode::BadRequest, "BadRequest" },
        HttpStatusCodeWithString{ services::HttpStatusCode::Unauthorized, "Unauthorized" },
        HttpStatusCodeWithString{ services::HttpStatusCode::PaymentRequired, "PaymentRequired" },
        HttpStatusCodeWithString{ services::HttpStatusCode::Forbidden, "Forbidden" },
        HttpStatusCodeWithString{ services::HttpStatusCode::NotFound, "NotFound" },
        HttpStatusCodeWithString{ services::HttpStatusCode::MethodNotAllowed, "MethodNotAllowed" },
        HttpStatusCodeWithString{ services::HttpStatusCode::NotAcceptable, "NotAcceptable" },
        HttpStatusCodeWithString{ services::HttpStatusCode::ProxyAuthenticationRequired, "ProxyAuthenticationRequired" },
        HttpStatusCodeWithString{ services::HttpStatusCode::RequestTimeOut, "RequestTimeOut" },
        HttpStatusCodeWithString{ services::HttpStatusCode::Conflict, "Conflict" },
        HttpStatusCodeWithString{ services::HttpStatusCode::Gone, "Gone" },
        HttpStatusCodeWithString{ services::HttpStatusCode::LengthRequired, "LengthRequired" },
        HttpStatusCodeWithString{ services::HttpStatusCode::PreconditionFailed, "PreconditionFailed" },
        HttpStatusCodeWithString{ services::HttpStatusCode::RequestEntityTooLarge, "RequestEntityTooLarge" },
        HttpStatusCodeWithString{ services::HttpStatusCode::RequestUriTooLarge, "RequestUriTooLarge" },
        HttpStatusCodeWithString{ services::HttpStatusCode::UnsupportedMediaType, "UnsupportedMediaType" },
        HttpStatusCodeWithString{ services::HttpStatusCode::RequestRangeNotSatisfiable, "RequestRangeNotSatisfiable" },
        HttpStatusCodeWithString{ services::HttpStatusCode::ExpectationFailed, "ExpectationFailed" },
        HttpStatusCodeWithString{ services::HttpStatusCode::InternalServerError, "InternalServerError" },
        HttpStatusCodeWithString{ services::HttpStatusCode::NotImplemented, "NotImplemented" },
        HttpStatusCodeWithString{ services::HttpStatusCode::BadGateway, "BadGateway" },
        HttpStatusCodeWithString{ services::HttpStatusCode::ServiceUnavailable, "ServiceUnavailable" },
        HttpStatusCodeWithString{ services::HttpStatusCode::GatewayTimeOut, "GatewayTimeOut" },
        HttpStatusCodeWithString{ services::HttpStatusCode::HttpVersionNotSupported, "HttpVersionNotSupported" }
    ));

class HttpClientTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{
public:
    HttpClientTest()
        : connector(connectionFactory)
        , connectionPtr(infra::UnOwnedSharedPtr(connection))
        , clientPtr(infra::UnOwnedSharedPtr(client))
    {
        EXPECT_CALL(factory, Hostname()).WillRepeatedly(testing::Return("localhost"));
        EXPECT_CALL(factory, Port()).WillRepeatedly(testing::Return(80));
        EXPECT_CALL(factory2, Hostname()).WillRepeatedly(testing::Return("localhost"));
        EXPECT_CALL(factory2, Port()).WillRepeatedly(testing::Return(80));
        EXPECT_CALL(connectionFactory, Connect(testing::Ref(connector)));
        connector.Connect(factory);
    }

    void Connect()
    {
        EXPECT_CALL(factory, ConnectionEstablished(testing::_)).WillOnce(infra::Lambda([this](infra::AutoResetFunction<void(infra::SharedPtr<services::HttpClientObserver> client)>& createdClient)
        {
            EXPECT_CALL(client, Connected());
            createdClient(clientPtr);
        }));

        connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
        {
            connectionObserver->Attach(connection);
            connection.SetOwnership(nullptr, connectionObserver);
            connectionObserver->Connected();
        });
    }

    testing::StrictMock<services::ConnectionFactoryWithNameLookupMock> connectionFactory;
    testing::StrictMock<services::HttpClientObserverFactoryMock> factory;
    testing::StrictMock<services::HttpClientObserverFactoryMock> factory2;
    services::HttpClientConnectorImpl<>::WithMaxHeaderSize<50> connector;
    testing::StrictMock<services::ConnectionStubWithAckReceivedMock> connection;
    infra::SharedPtr<services::Connection> connectionPtr;
    testing::StrictMock<services::HttpClientObserverMock> client;
    infra::SharedPtr<services::HttpClientObserver> clientPtr;
};

TEST_F(HttpClientTest, refused_connection_propagates_to_HttpClientFactory)
{
    EXPECT_CALL(factory, ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::refused));
    connector.ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::refused);
}

TEST_F(HttpClientTest, connection_failed_propagates_to_HttpClientFactory)
{
    EXPECT_CALL(factory, ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed));
    connector.ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::connectionAllocationFailed);
}

TEST_F(HttpClientTest, second_connection_is_tried_when_first_is_refused)
{
    connector.Connect(factory2);

    EXPECT_CALL(factory, ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::refused));
    EXPECT_CALL(connectionFactory, Connect(testing::Ref(connector)));
    connector.ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::refused);
}

TEST_F(HttpClientTest, second_connection_is_tried_when_first_is_cancelled)
{
    connector.Connect(factory2);

    EXPECT_CALL(connectionFactory, CancelConnect(testing::Ref(connector)));
    EXPECT_CALL(connectionFactory, Connect(testing::Ref(connector)));
    connector.CancelConnect(factory);
}

TEST_F(HttpClientTest, second_connection_is_removed_on_CancelConnect)
{
    connector.Connect(factory2);
    connector.CancelConnect(factory2);

    EXPECT_CALL(factory, ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed));
    connector.ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::connectionAllocationFailed);
}

TEST_F(HttpClientTest, second_connection_is_tried_when_first_is_closed)
{
    connector.Connect(factory2);

    Connect();
    EXPECT_CALL(connection, AbortAndDestroyMock());
    EXPECT_CALL(client, ClosingConnection());
    EXPECT_CALL(connectionFactory, Connect(testing::Ref(connector)));
    connection.AbortAndDestroy();
}

TEST_F(HttpClientTest, closed_connection_results_in_ClosingConnection)
{
    Connect();

    EXPECT_CALL(connection, AbortAndDestroyMock());
    EXPECT_CALL(client, ClosingConnection());
    connection.AbortAndDestroy();
}

TEST_F(HttpClientTest, Close_propagates_to_Connection)
{
    Connect();

    EXPECT_CALL(connection, CloseAndDestroyMock());
    EXPECT_CALL(client, ClosingConnection());
    client.Subject().Close();
}

TEST_F(HttpClientTest, AckReceived_propagates_to_Connection)
{
    Connect();

    EXPECT_CALL(connection, AckReceivedMock());
    client.Subject().AckReceived();
}

TEST_F(HttpClientTest, after_ConnectionEstablished_HttpClient_is_connected)
{
    EXPECT_CALL(factory, ConnectionEstablished(testing::_)).WillOnce(infra::Lambda([this](infra::AutoResetFunction<void(infra::SharedPtr<services::HttpClientObserver> client)>& createdClient)
    {
        EXPECT_CALL(client, Connected());
        createdClient(clientPtr);
    }));

    connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        connectionObserver->Attach(connection);
        connection.SetOwnership(nullptr, connectionObserver);
        connectionObserver->Connected();
    });
}

TEST_F(HttpClientTest, Get_request_is_executed)
{
    Connect();
    client.Subject().Get("/api/thing");

    ExecuteAllActions();

    EXPECT_EQ("GET /api/thing HTTP/1.1\r\nhost:localhost\r\n\r\n", connection.SentDataAsString());
}

TEST_F(HttpClientTest, Head_request_is_executed)
{
    Connect();
    client.Subject().Head("/api/thing");

    ExecuteAllActions();

    EXPECT_EQ("HEAD /api/thing HTTP/1.1\r\nhost:localhost\r\n\r\n", connection.SentDataAsString());
}

TEST_F(HttpClientTest, Connect_request_is_executed)
{
    Connect();
    client.Subject().Connect("/api/thing");

    ExecuteAllActions();
    EXPECT_EQ("CONNECT /api/thing HTTP/1.1\r\nhost:localhost\r\n\r\n", connection.SentDataAsString());
}

TEST_F(HttpClientTest, Options_request_is_executed)
{
    Connect();
    client.Subject().Options("/api/thing");

    ExecuteAllActions();
    EXPECT_EQ("OPTIONS /api/thing HTTP/1.1\r\nhost:localhost\r\n\r\n", connection.SentDataAsString());
}

TEST_F(HttpClientTest, Post_request_is_executed)
{
    Connect();
    client.Subject().Post("/api/thing", "body contents");

    ExecuteAllActions();
    EXPECT_EQ("POST /api/thing HTTP/1.1\r\nhost:localhost\r\ncontent-length:13\r\n\r\nbody contents", connection.SentDataAsString());
}

TEST_F(HttpClientTest, Put_request_is_executed)
{
    Connect();
    client.Subject().Put("/api/thing", "body contents");

    ExecuteAllActions();
    EXPECT_EQ("PUT /api/thing HTTP/1.1\r\nhost:localhost\r\ncontent-length:13\r\n\r\nbody contents", connection.SentDataAsString());
}

TEST_F(HttpClientTest, Patch_request_is_executed)
{
    Connect();
    client.Subject().Patch("/api/thing", "body contents");

    ExecuteAllActions();
    EXPECT_EQ("PATCH /api/thing HTTP/1.1\r\nhost:localhost\r\ncontent-length:13\r\n\r\nbody contents", connection.SentDataAsString());
}

TEST_F(HttpClientTest, Delete_request_is_executed)
{
    Connect();
    client.Subject().Delete("/api/thing", {});

    ExecuteAllActions();
    EXPECT_EQ("DELETE /api/thing HTTP/1.1\r\nhost:localhost\r\n\r\n", connection.SentDataAsString());
}

TEST_F(HttpClientTest, request_contains_correct_headers)
{
    Connect();
    std::array<services::HttpHeader, 2> headers{ services::HttpHeader{ "authorization", "Basic Y29ubmVjdGl2aXR5OmlzY29vbA==" }, { "connection", "close" } };

    client.Subject().Get("/api/thing", headers);

    ExecuteAllActions();
    EXPECT_EQ("GET /api/thing HTTP/1.1\r\nauthorization:Basic Y29ubmVjdGl2aXR5OmlzY29vbA==\r\nconnection:close\r\nhost:localhost\r\n\r\n", connection.SentDataAsString());
}

TEST_F(HttpClientTest, incorrect_response_version_should_not_call_StatusAvailable)
{
    Connect();

    EXPECT_CALL(client, StatusAvailable(testing::_)).Times(0);

    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(connection, AbortAndDestroyMock());
    EXPECT_CALL(client, ClosingConnection());
    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/X.Y 200 Success\r\n")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, incorrect_response_code_should_not_call_StatusAvailable)
{
    Connect();

    EXPECT_CALL(client, StatusAvailable(testing::_)).Times(0);

    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(connection, AbortAndDestroyMock());
    EXPECT_CALL(client, ClosingConnection());
    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 900 Invalid\r\n")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, response_with_supported_http_version_should_call_StatusAvailable)
{
    Connect();

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, BodyComplete());
    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.0 200 Success\r\nContent-Length:0\r\n\r\n")));
    ExecuteAllActions();

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, BodyComplete());
    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\nContent-Length:0\r\n\r\n")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, ResponseAvailable_contains_correct_status_code)
{
    Connect();

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::Continue));
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, BodyComplete());
    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 100 Continue\r\nContent-Length:0\r\n\r\n")));
    ExecuteAllActions();

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, BodyComplete());
    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\nContent-Length:0\r\n\r\n")));
    ExecuteAllActions();

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::BadRequest));
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, BodyComplete());
    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 400 Bad Request\r\nContent-Length:0\r\n\r\n")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, ResponseAvailable_contains_response_headers)
{
    Connect();

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Date", "Sat, 28 Nov 2009 04:36:25 GMT")));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Expires", "-1")));
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, BodyComplete());

    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\nDate:Sat, 28 Nov 2009 04:36:25 GMT\r\nExpires:-1\r\nContent-Length:0\r\n\r\n")));

    ExecuteAllActions();
}

TEST_F(HttpClientTest, too_long_header_is_rejected)
{
    Connect();

    EXPECT_CALL(client, StatusAvailable(testing::_));

    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(connection, AbortAndDestroyMock());
    EXPECT_CALL(client, ClosingConnection());
    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.0 200 Success\r\n012345678901234567890123456789012345678901234567890123456789\r\n")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, leading_spaces_are_stripped_from_header)
{
    Connect();

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Header", "Header Data")));
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, BodyComplete());

    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\nHeader:  Header Data\r\nContent-Length:0\r\n\r\n")));

    ExecuteAllActions();
}

TEST_F(HttpClientTest, handle_headers_without_content)
{
    Connect();

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Header", "")));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("AnotherHeader", "")));
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, BodyComplete());

    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\nHeader:  \r\nAnotherHeader:\r\nContent-Length:0\r\n\r\n")));

    ExecuteAllActions();
}

TEST_F(HttpClientTest, ResponseAvailable_forwards_response_body_to_client)
{
    Connect();

    EXPECT_CALL(connection, AckReceivedMock()).Times(2);
    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Date", "Sat, 28 Nov 2009 04:36:25 GMT")));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Expires", "-1")));
    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this](infra::SharedPtr<infra::StreamReader>& reader) {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        EXPECT_EQ("body\r\ndata", infra::ByteRangeAsString(stream.ContiguousRange()));
    }));
    EXPECT_CALL(client, BodyComplete());

    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\nDate:Sat, 28 Nov 2009 04:36:25 GMT\r\nExpires:-1\r\nContent-Length:10\r\n\r\nbody\r\ndata")));

    ExecuteAllActions();
}

TEST_F(HttpClientTest, ResponseAvailable_without_ContentLength_is_rejected)
{
    Connect();

    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(connection, AbortAndDestroyMock());
    EXPECT_CALL(client, ClosingConnection());

    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\n\r\nbody\r\ndata")));

    ExecuteAllActions();
}

TEST_F(HttpClientTest, response_in_parts_is_handled)
{
    Connect();

    client.Subject().Get("/");

    EXPECT_CALL(connection, AckReceivedMock()).Times(6);
    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\n")));
    ExecuteAllActions();

    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("Date:Sat, 28 Nov 2009 04:36:25 GMT\r")));

    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Date", "Sat, 28 Nov 2009 04:36:25 GMT")));
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("\n")));
    ExecuteAllActions();

    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Expires", "-1")));
    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this](infra::SharedPtr<infra::StreamReader>& reader) {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        EXPECT_EQ("body\r\ndat", infra::ByteRangeAsString(stream.ContiguousRange()));
    }));
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("Expires:-1\r\nContent-Length:10\r\n\r\nbody\r\ndat")));
    ExecuteAllActions();

    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this](infra::SharedPtr<infra::StreamReader>& reader) {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        EXPECT_EQ("a", infra::ByteRangeAsString(stream.ContiguousRange()));
    }));
    EXPECT_CALL(client, BodyComplete());
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("a")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, data_up_to_ContentLength_is_handled)
{
    Connect();

    client.Subject().Get("/");

    EXPECT_CALL(connection, AckReceivedMock()).Times(3);
    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Date", "Sat, 28 Nov 2009 04:36:25 GMT")));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Expires", "-1")));
    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this](infra::SharedPtr<infra::StreamReader>& reader) {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        EXPECT_EQ("body\r\nda", infra::ByteRangeAsString(stream.ContiguousRange()));
    }));
    EXPECT_CALL(client, BodyComplete());

    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\n")));
    ExecuteAllActions();
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("Date:Sat, 28 Nov 2009 04:36:25 GMT\r\nExpires:-1\r\nContent-Length:8\r\n\r\nbody\r\ndata")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, data_in_excess_of_ContentLength_is_ignored)
{
    Connect();

    EXPECT_CALL(connection, AckReceivedMock()).Times(2);
    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Date", "Sat, 28 Nov 2009 04:36:25 GMT")));
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Expires", "-1")));
    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this](infra::SharedPtr<infra::StreamReader>& reader) {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        EXPECT_EQ("body\r\ndata", infra::ByteRangeAsString(stream.ContiguousRange()));
    }));
    EXPECT_CALL(client, BodyComplete());

    client.Subject().Get("/");
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\nDate:Sat, 28 Nov 2009 04:36:25 GMT\r\nExpires:-1\r\nContent-Length:10\r\n\r\nbody\r\ndata")));
    ExecuteAllActions();

    EXPECT_CALL(connection, AbortAndDestroyMock());
    EXPECT_CALL(client, ClosingConnection());
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("extradata")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, Close_while_DataAvailable_is_handled)
{
    Connect();

    client.Subject().Get("/");

    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK)).WillOnce(infra::Lambda([this](services::HttpStatusCode result)
    {
        EXPECT_CALL(connection, CloseAndDestroyMock());
        EXPECT_CALL(client, ClosingConnection());
        client.Subject().Close();
    }));

    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\n")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, Close_while_BodyAvailable_is_handled)
{
    Connect();

    client.Subject().Get("/");

    EXPECT_CALL(connection, AckReceivedMock()).Times(3);
    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this](infra::SharedPtr<infra::StreamReader>& reader)
    {
        reader = nullptr;
        EXPECT_CALL(connection, CloseAndDestroyMock());
        EXPECT_CALL(client, ClosingConnection());
        client.Subject().Close();
    }));

    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\n")));
    ExecuteAllActions();
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("Content-Length:8\r\n\r\nbody\r\ndata")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, when_reader_is_stored_body_available_sends_same_pointer)
{
    Connect();

    client.Subject().Get("/");

    EXPECT_CALL(connection, AckReceivedMock()).Times(5);
    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\n")));
    ExecuteAllActions();

    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("Date:Sat, 28 Nov 2009 04:36:25 GMT\r")));

    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Date", "Sat, 28 Nov 2009 04:36:25 GMT")));
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("\n")));
    ExecuteAllActions();

    infra::SharedPtr<infra::StreamReader> readerPtr;
    EXPECT_CALL(client, HeaderAvailable(services::HttpHeader("Expires", "-1")));
    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this, &readerPtr](infra::SharedPtr<infra::StreamReader>& reader)
    {
        readerPtr = reader;
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        EXPECT_EQ("body\r\ndat", infra::ByteRangeAsString(stream.ContiguousRange()));
    }));
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("Expires:-1\r\nContent-Length:10\r\n\r\nbody\r\ndat")));
    ExecuteAllActions();

    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this](infra::SharedPtr<infra::StreamReader>& reader)
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::noFail);
        EXPECT_EQ("a", infra::ByteRangeAsString(stream.ContiguousRange()));
    }));
    EXPECT_CALL(client, BodyComplete());
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("a")));
    ExecuteAllActions();
}

TEST_F(HttpClientTest, closed_before_reader_is_reset)
{
    Connect();

    client.Subject().Get("/");

    auto clientConnection = connection.Observer();  // Keep the client alive so that reader may be kept alive a little longer

    EXPECT_CALL(connection, AckReceivedMock()).Times(2);
    EXPECT_CALL(client, StatusAvailable(services::HttpStatusCode::OK));
    EXPECT_CALL(client, BodyAvailable(testing::_)).WillOnce(infra::Lambda([this](infra::SharedPtr<infra::StreamReader>& reader)
    {
        EXPECT_CALL(connection, CloseAndDestroyMock());
        EXPECT_CALL(client, ClosingConnection());
        client.Subject().Close();
        reader = nullptr;
    }));

    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("HTTP/1.1 200 Success\r\n")));
    ExecuteAllActions();
    connection.SimulateDataReceived(infra::StringAsByteRange(infra::BoundedConstString("Content-Length:8\r\n\r\nbody\r\ndata")));
    ExecuteAllActions();
}
