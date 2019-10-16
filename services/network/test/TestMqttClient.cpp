#include "gmock/gmock.h"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/MqttClientImpl.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "services/network/test_doubles/MqttMock.hpp"
#include "network/test_doubles/AddressMock.hpp"

namespace testing
{
    struct DummyType {};

    template<>
    struct NiceMock<DummyType>
    {
        static void allow(const void* mock)
        {
            Mock::AllowUninterestingCalls(mock);
        }
    };

    typedef NiceMock<DummyType> UninterestingCalls;
}

class ConnectionStubWithSendStreamControl
    : public services::ConnectionStub
{
public:
    ~ConnectionStubWithSendStreamControl()
    {
        if (HasObserver())
            GetObserver().ClosingConnection();
    }

    virtual void RequestSendStream(std::size_t sendSize) override
    {
        if (autoSendStreamAvailable)
            services::ConnectionStub::RequestSendStream(sendSize);
        else
        {
            this->sendSize.Emplace(sendSize);
            RequestSendStreamMock(sendSize);
        }
    }

    virtual void AckReceived() override
    {
        if (mockAckReceived)
            AckReceivedMock();
        services::ConnectionStub::AckReceived();
    }

    void ScheduleGrantSendStream()
    {
        services::ConnectionStub::RequestSendStream(*sendSize);
        sendSize = infra::none;
    }

    void AutoSendStreamAvailableEnabled(bool newValue)
    {
        autoSendStreamAvailable = newValue;
    }

    void MockAckReceived(bool newValue)
    {
        mockAckReceived = newValue;
    }

    MOCK_METHOD0(AckReceivedMock, void());
    MOCK_METHOD1(RequestSendStreamMock, void(std::size_t));

private:
    bool autoSendStreamAvailable = true;
    bool mockAckReceived = false;
    infra::Optional<std::size_t> sendSize;
};

class MqttClientTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    MqttClientTest()
        : connector("clientId", "username", "password", "127.0.0.1", 1234, connectionFactory)
        , connectionPtr(infra::UnOwnedSharedPtr(connection))
        , clientPtr(infra::UnOwnedSharedPtr(client))
    {
        EXPECT_CALL(connectionFactory, Connect(testing::Ref(connector)));
        connector.Connect(factory);
    }

    ~MqttClientTest()
    {
        testing::UninterestingCalls().allow(&factory);
        testing::UninterestingCalls().allow(&client);
    }

    void Connect()
    {
        connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
        {
            connectionObserver->Attach(connection);
            connection.SetOwnership(nullptr, connectionObserver);
            connectionObserver->Connected();
        });

        ExecuteAllActions();

        EXPECT_CALL(factory, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([this](infra::AutoResetFunction<void(infra::SharedPtr<services::MqttClientObserver> client)>& createdClient)
        {
            EXPECT_CALL(client, Connected());
            createdClient(clientPtr);
        }));
        connection.SimulateDataReceived(std::vector<uint8_t>{ 0x20, 0x00, 0x00, 0x00 });
        ExecuteAllActions();

        connection.sentData.clear();
    }

    void FillTopic(infra::BoundedConstString topic)
    {
        EXPECT_CALL(client, FillTopic(testing::_)).WillRepeatedly(testing::Invoke([topic](infra::StreamWriter& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(writer);
            stream << topic;
        }));
    }

    void FillPayload(infra::BoundedConstString payload)
    {
        EXPECT_CALL(client, FillPayload(testing::_)).WillRepeatedly(testing::Invoke([payload](infra::StreamWriter& writer)
        {
            infra::TextOutputStream::WithErrorPolicy stream(writer);
            stream << payload;
        }));
    }

    void ExpectClosingConnection()
    {
        EXPECT_CALL(connection, AbortAndDestroyMock());
        EXPECT_CALL(client, ClosingConnection());
    }

    void ReceiveSubAck(uint16_t packetIdentifier, uint8_t returnCode)
    {
        connection.SimulateDataReceived(std::vector<uint8_t>{ 0x90, 0x03 });
        connection.SimulateDataReceived(std::vector<uint8_t>{ static_cast<uint8_t>(packetIdentifier >> 8), static_cast<uint8_t>(packetIdentifier), returnCode });
    }

    testing::StrictMock<services::ConnectionFactoryWithNameResolverMock> connectionFactory;
    testing::StrictMock<services::MqttClientObserverFactoryMock> factory;
    testing::StrictMock<services::MqttClientObserverMock> client;
    services::MqttClientConnectorImpl connector;
    testing::StrictMock<ConnectionStubWithSendStreamControl> connection;
    infra::SharedPtr<services::Connection> connectionPtr;
    infra::SharedPtr<services::MqttClientObserver> clientPtr;
};

TEST_F(MqttClientTest, refused_connection_propagates_to_MqttClientFactory)
{
    EXPECT_CALL(factory, ConnectionFailed(services::MqttClientObserverFactory::ConnectFailReason::refused));
    connector.ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::refused);
}

TEST_F(MqttClientTest, connection_failed_propagates_to_MqttClientFactory)
{
    EXPECT_CALL(factory, ConnectionFailed(services::MqttClientObserverFactory::ConnectFailReason::connectionAllocationFailed));
    connector.ConnectionFailed(services::ClientConnectionObserverFactoryWithNameResolver::ConnectFailReason::connectionAllocationFailed);
}

TEST_F(MqttClientTest, after_connected_connect_message_is_sent)
{
    connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        connectionObserver->Attach(connection);
        connection.SetOwnership(nullptr, connectionObserver);
        connectionObserver->Connected();
    });

    ExecuteAllActions();

    EXPECT_EQ((std::vector<uint8_t>{ 0x10, 0x28, 0x00, 0x04, 'M' , 'Q' , 'T' , 'T' , 0x04, 0xc2, 0x00, 0x00, 0x00, 0x08, 'c' , 'l' ,
        'i' , 'e' , 'n' , 't' , 'I' , 'd' , 0x00, 0x08, 'u' , 's' , 'e' , 'r' , 'n' , 'a' , 'm' , 'e' ,
        0x00, 0x08, 'p' , 'a' , 's' , 's' , 'w' , 'o' , 'r' , 'd' }), connection.sentData);
}

TEST_F(MqttClientTest, after_conack_MqttClient_is_connected)
{
    connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        connectionObserver->Attach(connection);
        connection.SetOwnership(nullptr, connectionObserver);
        connectionObserver->Connected();
    });

    ExecuteAllActions();

    EXPECT_CALL(factory, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([this](infra::AutoResetFunction<void(infra::SharedPtr<services::MqttClientObserver> client)>& createdClient)
    {
        EXPECT_CALL(client, Connected());
        createdClient(clientPtr);
    }));

    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x20, 0x00 });
    ExecuteAllActions();

    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x00, 0x00 });
    ExecuteAllActions();
}

TEST_F(MqttClientTest, partial_conack_is_ignored)
{
    connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        connectionObserver->Attach(connection);
        connection.SetOwnership(nullptr, connectionObserver);
        connectionObserver->Connected();
    });

    ExecuteAllActions();

    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x20 });
    ExecuteAllActions();

    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x00, 0x00 });
    ExecuteAllActions();
}

TEST_F(MqttClientTest, invalid_response_results_in_ConnectionFailed)
{
    connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        connectionObserver->Attach(connection);
        connection.SetOwnership(nullptr, connectionObserver);
        connectionObserver->Connected();
    });

    ExecuteAllActions();

    EXPECT_CALL(factory, ConnectionFailed(services::MqttClientObserverFactory::ConnectFailReason::initializationFailed));
    EXPECT_CALL(connection, AbortAndDestroyMock());
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x30, 0 });
    ExecuteAllActions();
}

TEST_F(MqttClientTest, timeout_results_in_ConnectionFailed)
{
    connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        connectionObserver->Attach(connection);
        connection.SetOwnership(nullptr, connectionObserver);
        connectionObserver->Connected();
    });

    ExecuteAllActions();

    EXPECT_CALL(factory, ConnectionFailed(services::MqttClientObserverFactory::ConnectFailReason::initializationTimedOut));
    EXPECT_CALL(connection, AbortAndDestroyMock());
    ForwardTime(std::chrono::minutes(1));
}

TEST_F(MqttClientTest, client_observer_allocation_failure_results_in_connection_aborted)
{
    connector.ConnectionEstablished([this](infra::SharedPtr<services::ConnectionObserver> connectionObserver)
    {
        connectionObserver->Attach(connection);
        connection.SetOwnership(nullptr, connectionObserver);
        connectionObserver->Connected();
    });

    ExecuteAllActions();

    EXPECT_CALL(factory, ConnectionEstablished(testing::_)).WillOnce(testing::Invoke([this](infra::AutoResetFunction<void(infra::SharedPtr<services::MqttClientObserver> client)>& createdClient)
    {
        createdClient(nullptr);
    }));
    EXPECT_CALL(connection, AbortAndDestroyMock());
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x20, 0x00, 0x00, 0x00 });
}

TEST_F(MqttClientTest, Publish_some_data)
{
    Connect();

    FillTopic("topic");
    FillPayload("payload");
    client.Subject().Publish();

    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0, 1, 'p', 'a', 'y', 'l', 'o', 'a', 'd' }), connection.sentData);

    EXPECT_CALL(client, PublishDone());
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x40, 0x02 });
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x00, 0x01 });
}

TEST_F(MqttClientTest, publish_aborts_connection_with_30_sec_timeout)
{
    Connect();

    FillTopic("topic");
    FillPayload("payload");
    client.Subject().Publish();

    ExecuteAllActions();

    ExpectClosingConnection();
    ForwardTime(std::chrono::seconds(30));
}

TEST_F(MqttClientTest, partial_puback_is_ignored)
{
    Connect();

    FillTopic("topic");
    FillPayload("payload");
    client.Subject().Publish();

    ExecuteAllActions();

    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x40, 0x02, 0x00 });
}

TEST_F(MqttClientTest, subscribe_to_a_topic)
{
    Connect();

    FillTopic("topic");
    client.Subject().Subscribe();

    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 0x82, 0x0a, 0, 1, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0x01}), connection.sentData);

    EXPECT_CALL(client, SubscribeDone());
    ReceiveSubAck(1, 0x01);
}

TEST_F(MqttClientTest, receive_suback_and_puback_back_to_back)
{
    Connect();

    connection.MockAckReceived(true);

    testing::InSequence seq;

    EXPECT_CALL(client, SubscribeDone());
    EXPECT_CALL(connection, AckReceivedMock());
    EXPECT_CALL(client, PublishDone());
    EXPECT_CALL(connection, AckReceivedMock());
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x90, 0x03, 0x00, 0x01, 0x01, 0x40, 0x02, 0x00, 0x01 });
}

TEST_F(MqttClientTest, suback_with_incorrect_packet_identifier_aborts_connection)
{
    Connect();

    FillTopic("topic");
    client.Subject().Subscribe();

    ExecuteAllActions();

    ExpectClosingConnection();
    ReceiveSubAck(2, 0x01);
}

TEST_F(MqttClientTest, rejected_subscription_aborts_connection)
{
    Connect();

    FillTopic("topic");
    client.Subject().Subscribe();

    ExecuteAllActions();

    ExpectClosingConnection();
    ReceiveSubAck(1, 0x80);
}

TEST_F(MqttClientTest, subscribe_aborts_connection_with_30_sec_timeout)
{
    Connect();

    FillTopic("topic");
    client.Subject().Subscribe();

    ExecuteAllActions();

    ExpectClosingConnection();
    ForwardTime(std::chrono::seconds(30));
}

TEST_F(MqttClientTest, closed_connection_results_in_ClosingConnection)
{
    Connect();

    ExpectClosingConnection();
    connection.AbortAndDestroy();
}

TEST_F(MqttClientTest, received_publish_is_forwarded_and_acked)
{
    Connect();

    connection.MockAckReceived(true);

    EXPECT_CALL(client, ReceivedNotification("topic", "payload"));
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c' });
    connection.SimulateDataReceived(std::vector<uint8_t>{ 1, 2, 'p', 'a', 'y', 'l', 'o', 'a', 'd' });

    EXPECT_CALL(connection, AckReceivedMock());
    client.Subject().NotificationDone();

    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 0x40, 0x02, 0x01, 0x02 }), connection.sentData);
}

TEST_F(MqttClientTest, received_data_are_not_processed_until_NotificationDone)
{
    Connect();

    connection.MockAckReceived(true);

    EXPECT_CALL(client, ReceivedNotification("topic", "payload"));
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 1, 2, 'p', 'a', 'y', 'l', 'o', 'a', 'd' });

    testing::Mock::VerifyAndClearExpectations(&connection);

    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 1, 2, 'p', 'a', 'y', 'l', 'o', 'a', 'd' });
}

TEST_F(MqttClientTest, received_data_are_not_processed_until_NotificationDone2)
{
    Connect();

    connection.MockAckReceived(true);

    EXPECT_CALL(client, ReceivedNotification("topic", "payload"));
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 1, 2, 'p', 'a', 'y', 'l', 'o', 'a', 'd' });

    testing::Mock::VerifyAndClearExpectations(&connection);

    ReceiveSubAck(1, 0x01);
}

TEST_F(MqttClientTest, receive_two_publishes)
{
    Connect();

    connection.MockAckReceived(true);

    EXPECT_CALL(client, ReceivedNotification("topic", "payload0"));
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x11, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 1, 2, 'p', 'a', 'y', 'l', 'o', 'a', 'd', '0' });

    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x11, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 1, 2, 'p', 'a', 'y', 'l', 'o', 'a', 'd', '1' });

    EXPECT_CALL(connection, AckReceivedMock());
    client.Subject().NotificationDone();

    EXPECT_CALL(client, ReceivedNotification("topic", "payload1"));
    ExecuteAllActions();
}

TEST_F(MqttClientTest, additional_received_data_are_processed_after_NotificationDone)
{
    Connect();

    EXPECT_CALL(client, ReceivedNotification("topic", "payload"));
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 1, 2, 'p', 'a', 'y', 'l', 'o', 'a', 'd' });

    ReceiveSubAck(1, 0x01);

    client.Subject().NotificationDone();

    EXPECT_CALL(client, SubscribeDone());
    ExecuteAllActions();
}

TEST_F(MqttClientTest, received_publish_acked_and_publish_can_be_interleaved)
{
    Connect();

    EXPECT_CALL(client, ReceivedNotification("topic", "payload"));
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0, 1, 'p', 'a', 'y', 'l', 'o', 'a', 'd' });

    connection.AutoSendStreamAvailableEnabled(false);

    EXPECT_CALL(connection, RequestSendStreamMock(testing::_)).Times(1);
    client.Subject().NotificationDone();
    client.Subject().Publish();

    connection.ScheduleGrantSendStream();

    EXPECT_CALL(connection, RequestSendStreamMock(testing::_)).Times(1);
    FillTopic("topic");
    FillPayload("payload");
    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 0x40, 0x02, 0x00, 0x01 }), connection.sentData);
    connection.sentData.clear();

    connection.ScheduleGrantSendStream();

    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0, 1, 'p', 'a', 'y', 'l', 'o', 'a', 'd' }), connection.sentData);
}

TEST_F(MqttClientTest, received_publish_acked_and_subscribe_can_be_interleaved)
{
    Connect();

    EXPECT_CALL(client, ReceivedNotification("topic", "payload"));
    connection.SimulateDataReceived(std::vector<uint8_t>{ 0x32, 0x10, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0, 1, 'p', 'a', 'y', 'l', 'o', 'a', 'd' });

    connection.AutoSendStreamAvailableEnabled(false);

    EXPECT_CALL(connection, RequestSendStreamMock(testing::_)).Times(1);
    client.Subject().NotificationDone();
    client.Subject().Subscribe();

    connection.ScheduleGrantSendStream();

    EXPECT_CALL(connection, RequestSendStreamMock(testing::_)).Times(1);
    FillTopic("topic");
    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 0x40, 0x02, 0x00, 0x01 }), connection.sentData);
    connection.sentData.clear();

    connection.ScheduleGrantSendStream();

    ExecuteAllActions();
    EXPECT_EQ((std::vector<uint8_t>{ 0x82, 0x0a, 0, 1, 0x00, 0x05, 't', 'o', 'p', 'i', 'c', 0x01}), connection.sentData);
}
