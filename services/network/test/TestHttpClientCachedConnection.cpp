#include "gmock/gmock.h"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/StdStringOutputStream.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/SharedOptional.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/HttpClientCachedConnection.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/HttpClientMock.hpp"
#include "services/util/Sha256MbedTls.hpp"

class HttpClientCachedConnectionTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    void ExpectHostnameAndPort(infra::BoundedConstString hostname = "host", uint16_t port = 10)
    {
        EXPECT_CALL(factory, Hostname()).WillOnce(testing::Return(hostname)).RetiresOnSaturation();
        EXPECT_CALL(factory, Port()).WillOnce(testing::Return(port)).RetiresOnSaturation();
    }

    void InitiateConnect(services::HttpClientObserverFactoryMock& factory, infra::BoundedConstString hostname = "host", uint16_t port = 10)
    {
        ExpectHostnameAndPort(hostname, port);

        connector.Connect(factory);
        ExecuteAllActions();
    }

    void InitiateConnectAndForwardToDelegate(services::HttpClientObserverFactoryMock& factory, infra::BoundedConstString hostname = "host", uint16_t port = 10)
    {
        EXPECT_CALL(connectorDelegate, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&connectingFactory));
        InitiateConnect(factory, hostname, port);
    }

    void CreateConnection(services::HttpClientObserverFactoryMock& factory, infra::BoundedConstString hostname = "host", uint16_t port = 10)
    {
        InitiateConnectAndForwardToDelegate(factory, hostname, port);

        EXPECT_CALL(factory, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([this](auto&& createdClientObserver)
            {
                auto observer = clientObserver.Emplace();
                EXPECT_CALL(*observer, Attached());
                createdClientObserver(observer);
            }));
        connectingFactory->ConnectionEstablished([this](infra::SharedPtr<services::HttpClientObserver> client) { clientSubject.Attach(client); });
    }

    void FinishRequest()
    {
        EXPECT_CALL(*clientObserver, BodyComplete());
        clientSubject.Observer().BodyComplete();
    }

    void CloseConnection()
    {
        FinishRequest();
        EXPECT_CALL(*clientObserver, Detaching());
        clientSubject.Detach();
    }

    infra::SharedOptional<testing::StrictMock<services::HttpClientObserverMock>> clientObserver;
    testing::StrictMock<services::HttpClientMock> clientSubject;
    testing::StrictMock<services::HttpClientConnectorMock> connectorDelegate;
    testing::StrictMock<services::HttpClientObserverFactoryMock> factory;
    testing::StrictMock<services::HttpClientObserverFactoryMock> factory2;

    services::Sha256MbedTls hasher;
    services::HttpClientCachedConnectionConnector connector{ connectorDelegate, hasher };
    services::HttpClientObserverFactory* connectingFactory = nullptr;
};

TEST_F(HttpClientCachedConnectionTest, normal_request_Get)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Get("target", testing::_));
    clientObserver->Subject().Get("target");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Head)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Head("target", testing::_));
    clientObserver->Subject().Head("target");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Connect)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Connect("target", testing::_));
    clientObserver->Subject().Connect("target");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Options)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Options("target", testing::_));
    clientObserver->Subject().Options("target");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Post1)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Post("target", "content", testing::_));
    clientObserver->Subject().Post("target", "content");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Post2)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Post("target", 10, testing::_));
    clientObserver->Subject().Post("target", 10);

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Post3)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Post("target", testing::_));
    clientObserver->Subject().Post("target");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Put1)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Put("target", "content", testing::_));
    clientObserver->Subject().Put("target", "content");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Put2)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Put("target", 10, testing::_));
    clientObserver->Subject().Put("target", 10);

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Put3)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Put("target", testing::_));
    clientObserver->Subject().Put("target");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Patch1)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Patch("target", "content", testing::_));
    clientObserver->Subject().Patch("target", "content");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Patch3)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Patch("target", testing::_));
    clientObserver->Subject().Patch("target");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, normal_request_Delete)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, Delete("target", "content", testing::_));
    clientObserver->Subject().Delete("target", "content");

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, other_normal_usage)
{
    CreateConnection(factory);

    EXPECT_CALL(clientSubject, AckReceived());
    clientObserver->Subject().AckReceived();

    services::ConnectionMock connection;
    EXPECT_CALL(clientSubject, GetConnection()).WillOnce(testing::ReturnRef(connection));
    EXPECT_EQ(&connection, &clientObserver->Subject().GetConnection());

    EXPECT_CALL(*clientObserver, Detaching());
    clientObserver->Subject().CloseConnection();

    clientSubject.Detach();
}

TEST_F(HttpClientCachedConnectionTest, all_responses)
{
    CreateConnection(factory);

    EXPECT_CALL(*clientObserver, StatusAvailable(services::HttpStatusCode::OK));
    clientSubject.Observer().StatusAvailable(services::HttpStatusCode::OK);

    EXPECT_CALL(*clientObserver, HeaderAvailable(services::HttpHeader("field", "value")));
    clientSubject.Observer().HeaderAvailable(services::HttpHeader("field", "value"));

    infra::SharedOptional<infra::StreamReaderMock> reader;
    EXPECT_CALL(*clientObserver, BodyAvailable(testing::_));
    clientSubject.Observer().BodyAvailable(reader.Emplace());

    infra::StreamWriterMock writer;
    infra::AccessedBySharedPtr writerAccess(infra::emptyFunction);
    EXPECT_CALL(*clientObserver, SendStreamAvailable(testing::_));
    clientSubject.Observer().SendStreamAvailable(writerAccess.MakeShared(writer));

    EXPECT_CALL(*clientObserver, FillContent(testing::_));
    clientSubject.Observer().FillContent(writer);

    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, connection_failed)
{
    InitiateConnectAndForwardToDelegate(factory);

    EXPECT_CALL(factory, ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed));
    connectingFactory->ConnectionFailed(services::HttpClientObserverFactory::ConnectFailReason::connectionAllocationFailed);
}

TEST_F(HttpClientCachedConnectionTest, cancel_connecting_factory)
{
    InitiateConnectAndForwardToDelegate(factory);

    EXPECT_CALL(connectorDelegate, CancelConnect(testing::Ref(*connectingFactory)));
    connector.CancelConnect(factory);
}

TEST_F(HttpClientCachedConnectionTest, cancel_waiting_factory)
{
    InitiateConnectAndForwardToDelegate(factory);

    connector.Connect(factory2);
    ExecuteAllActions();

    connector.CancelConnect(factory2);
}

TEST_F(HttpClientCachedConnectionTest, get_hostname_and_port)
{
    InitiateConnectAndForwardToDelegate(factory);

    EXPECT_CALL(factory, Hostname()).WillRepeatedly(testing::Return("host"));
    EXPECT_CALL(factory, Port()).WillRepeatedly(testing::Return(80));
    EXPECT_EQ("host", connectingFactory->Hostname());
    EXPECT_EQ(80, connectingFactory->Port());
}

TEST_F(HttpClientCachedConnectionTest, second_request_on_same_connection)
{
    CreateConnection(factory);
    FinishRequest();
    EXPECT_CALL(*clientObserver, Detaching());
    clientObserver->Detach();

    EXPECT_CALL(factory, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([this](auto&& createdClientObserver)
        {
            auto observer = clientObserver.Emplace();
            EXPECT_CALL(*observer, Attached());
            createdClientObserver(observer);
        }));
    InitiateConnect(factory, "host", 10);
    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, second_request_but_for_different_host)
{
    CreateConnection(factory);
    FinishRequest();
    EXPECT_CALL(*clientObserver, Detaching());
    clientObserver->Detach();

    ExpectHostnameAndPort("host2", 10);
    EXPECT_CALL(clientSubject, CloseConnection()).WillOnce([this]() { clientSubject.Detach(); });
    CreateConnection(factory, "host2", 10);
    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, second_request_but_for_different_port)
{
    CreateConnection(factory);
    FinishRequest();
    EXPECT_CALL(*clientObserver, Detaching());
    clientObserver->Detach();

    ExpectHostnameAndPort("host", 9);
    EXPECT_CALL(clientSubject, CloseConnection()).WillOnce([this]() { clientSubject.Detach(); });
    CreateConnection(factory, "host", 9);
    CloseConnection();
}

TEST_F(HttpClientCachedConnectionTest, connection_times_out)
{
    CreateConnection(factory);
    FinishRequest();
    EXPECT_CALL(*clientObserver, Detaching());
    clientObserver->Detach();

    EXPECT_CALL(clientSubject, CloseConnection()).WillOnce([this]() { clientSubject.Detach(); });
    ForwardTime(std::chrono::minutes(1));
}

TEST_F(HttpClientCachedConnectionTest, connection_is_closed_during_StatusAvailable)
{
    CreateConnection(factory);

    EXPECT_CALL(*clientObserver, StatusAvailable(services::HttpStatusCode::BadRequest)).WillOnce(testing::Invoke([this](services::HttpStatusCode)
        {
            EXPECT_CALL(*clientObserver, Detaching());
            clientObserver->Detach();
        }));
    clientSubject.Observer().StatusAvailable(services::HttpStatusCode::BadRequest);

    clientSubject.Observer().HeaderAvailable(services::HttpHeader("field", "value"));

    infra::SharedOptional<infra::StreamReaderMock> reader;
    clientSubject.Observer().BodyAvailable(reader.Emplace());

    clientSubject.Observer().BodyComplete();

    clientSubject.Detach();
}

TEST_F(HttpClientCachedConnectionTest, Close_is_forwarded_to_client)
{
    CreateConnection(factory);

    EXPECT_CALL(*clientObserver, CloseRequested());
    clientSubject.Observer().CloseRequested();

    EXPECT_CALL(*clientObserver, Detaching());
    clientSubject.Detach();
}

TEST_F(HttpClientCachedConnectionTest, after_Close_when_no_client_is_connected_connection_is_closed_immediately)
{
    CreateConnection(factory);

    FinishRequest();
    EXPECT_CALL(*clientObserver, Detaching());
    clientObserver->Detach();

    EXPECT_CALL(clientSubject, CloseConnection()).WillOnce([this]() { clientSubject.Detach(); });
    clientSubject.Observer().CloseRequested();
}

TEST_F(HttpClientCachedConnectionTest, after_handling_one_close_request_next_request_obeys_timeout)
{
    CreateConnection(factory);

    FinishRequest();
    EXPECT_CALL(*clientObserver, Detaching());
    clientObserver->Detach();

    EXPECT_CALL(clientSubject, CloseConnection()).WillOnce([this]() { clientSubject.Detach(); });
    clientSubject.Observer().CloseRequested();

    // New request

    CreateConnection(factory);
    FinishRequest();
    EXPECT_CALL(*clientObserver, Detaching());
    clientObserver->Detach();

    EXPECT_CALL(clientSubject, CloseConnection()).WillOnce([this]() { clientSubject.Detach(); });
    ForwardTime(std::chrono::minutes(1));
}
