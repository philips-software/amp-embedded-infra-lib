#include "gmock/gmock.h"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/ExclusiveStartingConnection.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"

class ExclusiveStartingConnectionTest
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
            connection->Attach(observer);
        });
    }

    void ExpectConnectionEstablished(services::ClientConnectionObserverFactoryMock& factory)
    {
        EXPECT_CALL(factory, ConnectionEstablishedMock(testing::_)).WillOnce(testing::Invoke([this](infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
        {
            auto observer = connectionObserver.Emplace();
            EXPECT_CALL(*observer, Attached());
            createdObserver(observer);
        }));
    }

    void ExpectConnectionEstablishedThatIsImmediatelyClosed(services::ClientConnectionObserverFactoryMock& factory)
    {
        EXPECT_CALL(factory, ConnectionEstablishedMock(testing::_)).WillOnce(testing::Invoke([this](infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
        {
            auto observer = connectionObserver.Emplace();
            EXPECT_CALL(*observer, Attached());
            EXPECT_CALL(*observer, Close());
            createdObserver(observer);
        }));
    }

    void CreateClientConnection(services::ConnectionFactory& factory)
    {
        EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult));
        factory.Connect(clientFactory);
        ExecuteAllActions();

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
            EXPECT_CALL(*observer, Attached());
            createdObserver(observer);
        }));
        serverResult->ConnectionAccepted([this](infra::SharedPtr<services::ConnectionObserver> observer)
        {
            auto connectionPtr = connection.Emplace();
            connection->SetOwnership(connectionPtr, observer);
            connection->Attach(observer);
        }, services::IPv4AddressLocalHost());
        ExecuteAllActions();
    }

    testing::StrictMock<services::ConnectionFactoryMock> connectionFactory;
    services::ExclusiveStartingConnectionFactoryMutex::WithMaxConnections<2> mutex;
    services::ExclusiveStartingConnectionFactory::WithListenersAndConnectors<2, 3> exclusive{ mutex, connectionFactory };

    testing::StrictMock<services::ClientConnectionObserverFactoryMock> clientFactory;
    services::ClientConnectionObserverFactory* clientResult = nullptr;

    testing::StrictMock<services::ServerConnectionObserverFactoryMock> serverFactory;
    services::ServerConnectionObserverFactory* serverResult = nullptr;
    infra::SharedOptional<int> listenerStorage;

    infra::SharedOptional<testing::StrictMock<services::ConnectionObserverFullMock>> connectionObserver;
    infra::SharedOptional<testing::StrictMock<services::ConnectionWithHostnameMock>> connection;
};

TEST_F(ExclusiveStartingConnectionTest, construct_one_connection_via_Connect)
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
    connection->Observer().SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>());
    EXPECT_CALL(*connectionObserver, DataReceived());
    connection->Observer().DataReceived();
    EXPECT_CALL(*connectionObserver, Close());
    connection->Observer().Close();
    EXPECT_CALL(*connectionObserver, Abort());
    connection->Observer().Abort();

    EXPECT_CALL(*connectionObserver, Detaching());
    connection->ResetOwnership();
    EXPECT_CALL(clientFactory, Destructor);
}

TEST_F(ExclusiveStartingConnectionTest, construct_one_connection_but_fails)
{
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult));
    exclusive.Connect(clientFactory);
    ExecuteAllActions();

    EXPECT_CALL(clientFactory, ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::refused));
    clientResult->ConnectionFailed(services::ClientConnectionObserverFactory::ConnectFailReason::refused);
    EXPECT_CALL(clientFactory, Destructor);
}

TEST_F(ExclusiveStartingConnectionTest, construct_one_connection_and_cancel)
{
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult));
    exclusive.Connect(clientFactory);
    ExecuteAllActions();

    EXPECT_CALL(connectionFactory, CancelConnect(testing::Ref(*clientResult)));
    exclusive.CancelConnect(clientFactory);
    EXPECT_CALL(clientFactory, Destructor);
}

TEST_F(ExclusiveStartingConnectionTest, construct_one_connection_via_Listen)
{
    auto listener = Listen(exclusive);
    CreateServerConnection();

    EXPECT_CALL(*connectionObserver, Detaching());
    connection->ResetOwnership();
    EXPECT_CALL(clientFactory, Destructor);
}

TEST_F(ExclusiveStartingConnectionTest, constructing_second_connection_waits_for_data_on_first_connection)
{
    // Build
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult));
    exclusive.Connect(clientFactory);
    ExecuteAllActions();

    ExpectConnectionEstablished(clientFactory);
    ConnectionEstablished(*clientResult);

    // Operate
    services::ClientConnectionObserverFactory* clientResult2 = nullptr;
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult2));
    exclusive.Connect(clientFactory);
    ExecuteAllActions();

    infra::SharedOptional<testing::StrictMock<services::ConnectionObserverFullMock>> connectionObserver2;
    infra::SharedOptional<testing::StrictMock<services::ConnectionWithHostnameMock>> connection2;

    clientResult2->ConnectionEstablished([&](infra::SharedPtr<services::ConnectionObserver> observer)
    {
        auto connectionPtr = connection2.Emplace();
        connection2->SetOwnership(connectionPtr, observer);
        connection2->Attach(observer);
    });

    // After receiving data on the first connection, the second connection is allowed to start
    EXPECT_CALL(clientFactory, ConnectionEstablishedMock(testing::_)).WillOnce(testing::Invoke([&](infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
    {
        auto observer = connectionObserver2.Emplace();
        EXPECT_CALL(*observer, Attached());
        createdObserver(observer);
    }));
    EXPECT_CALL(*connectionObserver, DataReceived());
    connection->Observer().DataReceived();

    //Destroy
    EXPECT_CALL(*connectionObserver, Detaching());
    connection->ResetOwnership();
    EXPECT_CALL(*connectionObserver2, Detaching());
    connection2->ResetOwnership();
    EXPECT_CALL(clientFactory, Destructor);
}

TEST_F(ExclusiveStartingConnectionTest, constructing_second_connection_waits_for_removal_of_first_connection)
{
    // Build
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult));
    exclusive.Connect(clientFactory);
    ExecuteAllActions();

    ExpectConnectionEstablished(clientFactory);
    ConnectionEstablished(*clientResult);

    // Operate
    services::ClientConnectionObserverFactory* clientResult2 = nullptr;
    EXPECT_CALL(connectionFactory, Connect(testing::_)).WillOnce(infra::SaveRef<0>(&clientResult2));
    exclusive.Connect(clientFactory);
    ExecuteAllActions();

    infra::SharedOptional<testing::StrictMock<services::ConnectionObserverFullMock>> connectionObserver2;
    infra::SharedOptional<testing::StrictMock<services::ConnectionWithHostnameMock>> connection2;

    clientResult2->ConnectionEstablished([&](infra::SharedPtr<services::ConnectionObserver> observer)
    {
        auto connectionPtr = connection2.Emplace();
        connection2->SetOwnership(connectionPtr, observer);
        connection2->Attach(observer);
    });

    // After receiving data on the first connection, the second connection is allowed to start
    EXPECT_CALL(clientFactory, ConnectionEstablishedMock(testing::_)).WillOnce(testing::Invoke([&](infra::Function<void(infra::SharedPtr<services::ConnectionObserver> connectionObserver)> createdObserver)
    {
        auto observer = connectionObserver2.Emplace();
        EXPECT_CALL(*observer, Attached());
        createdObserver(observer);
    }));
    EXPECT_CALL(*connectionObserver, Detaching());
    connection->ResetOwnership();

    //Destroy
    EXPECT_CALL(*connectionObserver2, Detaching());
    connection2->ResetOwnership();
    EXPECT_CALL(clientFactory, Destructor);
}
