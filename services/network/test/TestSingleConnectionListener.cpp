#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/SingleConnectionListener.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "gmock/gmock.h"

class SingleConnectionListenerTest
    : public testing::Test
{
public:
    class ConnectionObserverMock
        : public services::ConnectionObserverMock
    {
    public:
        MOCK_METHOD(void, Constructed, (services::IPAddress address), ());
        MOCK_METHOD(void, Destructed, (), ());
    };

    class NewConnectionStrategyMock
        : public services::NewConnectionStrategy
    {
    public:
        MOCK_METHOD(void, StopCurrentConnection, (services::SingleConnectionListener& listener), (override));
        MOCK_METHOD(void, StartNewConnection, (), (override));
    };

    testing::StrictMock<ConnectionObserverMock> connectionObserverMock;

    class ConnectionObserverStorage
        : public services::ConnectionObserver
    {
    public:
        ConnectionObserverStorage(ConnectionObserverMock& base, services::IPAddress address)
            : base(base)
        {
            base.Constructed(address);
        }

        ~ConnectionObserverStorage()
        {
            base.Destructed();
        }

        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& stream) override
        {
            base.SendStreamAvailable(std::move(stream));
        }

        void DataReceived() override
        {
            base.DataReceived();
        }

    private:
        ConnectionObserverMock& base;
    };

    void ConnectionAccepted()
    {
        serverConnectionObserverFactory->ConnectionAccepted(
            [this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
            {
                connection.Attach(connectionObserver);
                connectionAccepted.callback(connectionObserver);
            },
            services::IPAddress{ services::IPv4Address{ 1, 2, 3, 4 } });
    }

    void AcceptConnection()
    {
        EXPECT_CALL(connectionObserverMock, Constructed(services::IPAddress{ services::IPv4Address{ 1, 2, 3, 4 } }));
        EXPECT_CALL(connectionAccepted, callback(testing::_)).WillOnce(testing::SaveArg<0>(&connectionObserver));
        ConnectionAccepted();
    }

    infra::Creator<services::ConnectionObserver, ConnectionObserverStorage, void(services::IPAddress address)> connectionObserverCreator{ [this](infra::Optional<ConnectionObserverStorage>& connectionObserver, services::IPAddress address)
        {
            connectionObserver.Emplace(connectionObserverMock, address);
        } };

    testing::StrictMock<services::ConnectionFactoryMock> connectionFactory;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(connectionFactory, Listen(1, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr)));
        } };
    services::SingleConnectionListener listener{ connectionFactory, 1, { connectionObserverCreator } };
    infra::MockCallback<void(infra::SharedPtr<services::ConnectionObserver>)> connectionAccepted;
    infra::SharedPtr<services::ConnectionObserver> connectionObserver;
    testing::StrictMock<services::ConnectionMock> connection;
    testing::StrictMock<NewConnectionStrategyMock> newConnectionStrategy;
};

TEST_F(SingleConnectionListenerTest, create_connection)
{
    AcceptConnection();

    EXPECT_CALL(connectionObserverMock, Destructed());
    connectionObserver = nullptr;
}

TEST_F(SingleConnectionListenerTest, second_connection_cancels_first)
{
    AcceptConnection();

    EXPECT_CALL(connection, CloseAndDestroy()).WillOnce(testing::Invoke([this]()
        {
            connectionObserver = nullptr;
        }));
    ConnectionAccepted();

    EXPECT_CALL(connectionObserverMock, Destructed());
    EXPECT_CALL(connectionObserverMock, Constructed(services::IPAddress{ services::IPv4Address{ 1, 2, 3, 4 } }));
    EXPECT_CALL(connectionAccepted, callback(testing::_));
    connection.Detach();

    EXPECT_CALL(connectionObserverMock, Destructed());
}

TEST_F(SingleConnectionListenerTest, second_connection_while_first_is_destroyed_but_not_allocatable)
{
    AcceptConnection();

    infra::WeakPtr<services::ConnectionObserver> weakObserver = connectionObserver;
    EXPECT_CALL(connectionObserverMock, Destructed());
    connection.Detach();
    connectionObserver = nullptr;

    ConnectionAccepted();

    EXPECT_CALL(connectionObserverMock, Constructed(services::IPAddress{ services::IPv4Address{ 1, 2, 3, 4 } }));
    EXPECT_CALL(connectionAccepted, callback(testing::_));
    weakObserver = nullptr;

    EXPECT_CALL(connectionObserverMock, Destructed());
}

TEST_F(SingleConnectionListenerTest, third_connection_while_second_has_not_yet_been_made)
{
    AcceptConnection();

    infra::WeakPtr<services::ConnectionObserver> weakObserver = connectionObserver;
    EXPECT_CALL(connectionObserverMock, Destructed());
    connection.Detach();
    connectionObserver = nullptr;

    serverConnectionObserverFactory->ConnectionAccepted(
        [this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
        {
            EXPECT_EQ(nullptr, connectionObserver);
        },
        services::IPAddress{ services::IPv4Address{ 1, 2, 3, 4 } });

    ConnectionAccepted();

    EXPECT_CALL(connectionObserverMock, Constructed(services::IPAddress{ services::IPv4Address{ 1, 2, 3, 4 } }));
    EXPECT_CALL(connectionAccepted, callback(testing::_)).WillOnce(testing::SaveArg<0>(&connectionObserver));
    weakObserver = nullptr;

    EXPECT_CALL(connectionObserverMock, Destructed());
    connectionObserver = nullptr;
}

TEST_F(SingleConnectionListenerTest, Stop_aborts_connection)
{
    AcceptConnection();
    connectionObserver = nullptr;

    infra::MockCallback<void()> onDone;
    EXPECT_CALL(connection, AbortAndDestroy());
    listener.Stop([&onDone]()
        {
            onDone.callback();
        });

    EXPECT_CALL(connectionObserverMock, Destructed());
    EXPECT_CALL(onDone, callback());
    connection.Detach();
}

TEST_F(SingleConnectionListenerTest, NewConnectionStrategy)
{
    listener.SetNewConnectionStrategy(newConnectionStrategy);

    EXPECT_CALL(newConnectionStrategy, StopCurrentConnection(testing::Ref(listener)));
    AcceptConnection();
    EXPECT_CALL(newConnectionStrategy, StartNewConnection());
    listener.StopCurrentConnection(listener);
    listener.StartNewConnection();
    connectionObserver = nullptr;

    EXPECT_CALL(newConnectionStrategy, StopCurrentConnection(testing::Ref(listener)));
    ConnectionAccepted();

    EXPECT_CALL(connection, CloseAndDestroy());
    listener.StopCurrentConnection(listener);

    EXPECT_CALL(connectionObserverMock, Destructed());
    EXPECT_CALL(newConnectionStrategy, StartNewConnection());
    connection.Detach();

    EXPECT_CALL(connectionObserverMock, Constructed(services::IPAddress{ services::IPv4Address{ 1, 2, 3, 4 } }));
    EXPECT_CALL(connectionAccepted, callback(testing::_));
    listener.StartNewConnection();

    EXPECT_CALL(connectionObserverMock, Destructed());
}
