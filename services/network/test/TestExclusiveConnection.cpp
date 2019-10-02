#include "gmock/gmock.h"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/ExclusiveConnection.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"

class ExclusiveConnectionTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    void ConnectionEstablished(services::ClientConnectionObserverFactory& factory)
    {
        factory.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> observer)
        {
            auto connectionPtr = connection.Emplace();
            connection->SetOwnership(connectionPtr, observer);
            observer->Attach(*connection);
            observer->Connected();
        });
    }

    void ExpectConnectionEstablished(services::ClientConnectionObserverFactoryMock& factory)
    {
        EXPECT_CALL(factory, ConnectionEstablishedMock(testing::_)).WillOnce(testing::Invoke([this](infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
        {
            auto observer = connectionObserver.Emplace();
            EXPECT_CALL(*observer, Connected());
            createdObserver(observer);
        }));
    }

    void ExpectConnectionEstablishedThatIsImmediatelyClosed(services::ClientConnectionObserverFactoryMock& factory)
    {
        EXPECT_CALL(factory, ConnectionEstablishedMock(testing::_)).WillOnce(testing::Invoke([this](infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
        {
            auto observer = connectionObserver.Emplace();
            EXPECT_CALL(*observer, Connected());
            EXPECT_CALL(*observer, Close());
            createdObserver(observer);
        }));
    }

    void CreateClientConnection(services::ConnectionFactory& factory)
    {
        EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult));
        factory.Connect(clientFactory);

        EXPECT_CALL(clientFactory, Address()).WillOnce(testing::Return(services::IPv4AddressLocalHost()));
        EXPECT_EQ(services::IPAddress(services::IPv4AddressLocalHost()), clientResult->Address());

        EXPECT_CALL(clientFactory, Port()).WillOnce(testing::Return(12));
        EXPECT_EQ(12, clientResult->Port());

        ExpectConnectionEstablished(clientFactory);
        ConnectionEstablished(*clientResult);
        ExecuteAllActions();
    }

    infra::SharedPtr<void> Listen(services::ConnectionFactory& factory)
    {
        EXPECT_CALL(connectionFactory, Listen(14, testing::_, services::IPVersions::both)).WillOnce(testing::Invoke([this](uint16_t port, services::ServerConnectionObserverFactory& factory, services::IPVersions versions)
        {
            EXPECT_EQ(14, port);
            serverResult = &factory;
            return listenerStorage.Emplace();
        }));
        return factory.Listen(14, serverFactory);
    }

    void CreateServerConnection()
    {
        EXPECT_CALL(serverFactory, ConnectionAcceptedMock(testing::_, services::IPAddress(services::IPv4AddressLocalHost())))
            .WillOnce(testing::Invoke([this](infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver, services::IPAddress address)
        {
            auto observer = connectionObserver.Emplace();
            EXPECT_CALL(*observer, Connected());
            createdObserver(observer);
        }));
        serverResult->ConnectionAccepted([this](infra::SharedPtr<services::ConnectionObserver> observer)
        {
            auto connectionPtr = connection.Emplace();
            connection->SetOwnership(connectionPtr, observer);
            observer->Attach(*connection);
            observer->Connected();
        }, services::IPv4AddressLocalHost());
        ExecuteAllActions();
    }

    testing::StrictMock<services::ConnectionFactoryMock> connectionFactory;
    services::ExclusiveConnectionFactoryMutex mutex;
    services::ExclusiveConnectionFactory::WithListenersAndConnectors<2, 3> exclusive{ mutex, connectionFactory, true };
    services::ExclusiveConnectionFactory::WithListenersAndConnectors<2, 3> exclusive_dont_close{ mutex, connectionFactory, false };

    testing::StrictMock<services::ClientConnectionObserverFactoryMock> clientFactory;
    services::ClientConnectionObserverFactory* clientResult = nullptr;

    testing::StrictMock<services::ServerConnectionObserverFactoryMock> serverFactory;
    services::ServerConnectionObserverFactory* serverResult = nullptr;
    infra::SharedOptional<int> listenerStorage;

    infra::SharedOptional<testing::StrictMock<services::ConnectionObserverFullMock>> connectionObserver;
    infra::SharedOptional<testing::StrictMock<services::ConnectionWithHostnameMock>> connection;
};

TEST_F(ExclusiveConnectionTest, construct_one_connection_via_Connect)
{
    CreateClientConnection(exclusive);

    EXPECT_CALL(*connection, RequestSendStream(10));
    connectionObserver->Subject().RequestSendStream(10);
    EXPECT_CALL(*connection, MaxSendStreamSize()).WillOnce(testing::Return(123));
    EXPECT_EQ(123, connectionObserver->Subject().MaxSendStreamSize());
    EXPECT_CALL(*connection, ReceiveStream());
    connectionObserver->Subject().ReceiveStream();
    EXPECT_CALL(*connection, AckReceived());
    connectionObserver->Subject().AckReceived();
    EXPECT_CALL(*connection, CloseAndDestroy());
    connectionObserver->Subject().CloseAndDestroy();
    EXPECT_CALL(*connection, AbortAndDestroy());
    connectionObserver->Subject().AbortAndDestroy();
    EXPECT_CALL(*connection, SetHostname("hostname"));
    static_cast<services::ConnectionWithHostname&>(connectionObserver->Subject()).SetHostname("hostname");
    EXPECT_CALL(*connectionObserver, SendStreamAvailable(testing::_));
    connection->GetObserver().SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>());
    EXPECT_CALL(*connectionObserver, DataReceived());
    connection->GetObserver().DataReceived();
    EXPECT_CALL(*connectionObserver, Close());
    connection->GetObserver().Close();
    EXPECT_CALL(*connectionObserver, Abort());
    connection->GetObserver().Abort();

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();
}

TEST_F(ExclusiveConnectionTest, construct_one_connection_but_fails)
{
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult));
    exclusive.Connect(clientFactory);

    EXPECT_CALL(clientFactory, ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::refused));
    clientResult->ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::refused);
}

TEST_F(ExclusiveConnectionTest, construct_one_connection_and_cancel)
{
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult));
    exclusive.Connect(clientFactory);

    EXPECT_CALL(connectionFactory, CancelConnect(testing::Ref(*clientResult)));
    exclusive.CancelConnect(clientFactory);
}

TEST_F(ExclusiveConnectionTest, construct_one_connection_via_Listen)
{
    auto listener = Listen(exclusive);
    CreateServerConnection();

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();
}

TEST_F(ExclusiveConnectionTest, constructing_second_connection_results_in_close_on_first_connection)
{
    CreateClientConnection(exclusive);

    testing::StrictMock<services::ClientConnectionObserverFactoryMock> clientFactory2;
    services::ClientConnectionObserverFactory* clientResult2 = nullptr;
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult2));
    exclusive.Connect(clientFactory2);

    EXPECT_CALL(*connectionObserver, Close());
    ConnectionEstablished(*clientResult2);

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();

    ExpectConnectionEstablished(clientFactory2);
    ExecuteAllActions();

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();
}

TEST_F(ExclusiveConnectionTest, constructing_second_connection_does_not_result_in_close_on_first_connection)
{
    CreateClientConnection(exclusive_dont_close);

    testing::StrictMock<services::ClientConnectionObserverFactoryMock> clientFactory2;
    services::ClientConnectionObserverFactory* clientResult2 = nullptr;
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult2));
    exclusive.Connect(clientFactory2);

    ConnectionEstablished(*clientResult2);

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();

    ExpectConnectionEstablished(clientFactory2);
    ExecuteAllActions();

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();
}

TEST_F(ExclusiveConnectionTest, second_connection_is_immediately_requested_to_close_if_third_is_waiting)
{
    CreateClientConnection(exclusive);

    // second
    testing::StrictMock<services::ClientConnectionObserverFactoryMock> clientFactory2;
    services::ClientConnectionObserverFactory* clientResult2 = nullptr;
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult2));
    exclusive.Connect(clientFactory2);

    EXPECT_CALL(*connectionObserver, Close());
    ConnectionEstablished(*clientResult2);

    // third
    testing::StrictMock<services::ClientConnectionObserverFactoryMock> clientFactory3;
    services::ClientConnectionObserverFactory* clientResult3 = nullptr;
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult3));
    exclusive.Connect(clientFactory3);

    EXPECT_CALL(*connectionObserver, Close());
    ConnectionEstablished(*clientResult3);

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();

    ExpectConnectionEstablishedThatIsImmediatelyClosed(clientFactory2);
    ExecuteAllActions();

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();

    ExpectConnectionEstablished(clientFactory3);
    ExecuteAllActions();

    EXPECT_CALL(*connectionObserver, ClosingConnection());
    connection->ResetOwnership();
}
