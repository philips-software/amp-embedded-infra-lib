#include "gmock/gmock.h"
#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/SerialServer.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"

class SerialServerTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    SerialServerTest()
    {
        connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, *connection, services::IPv4AddressLocalHost());
    }

    ~SerialServerTest()
    {
        if (connectionPtr != nullptr)
        {
            EXPECT_CALL(*connection, AbortAndDestroyMock);
            connection->AbortAndDestroy();
        }
    }

    infra::SharedOptional<testing::StrictMock<services::ConnectionStub>> connection;
    infra::SharedPtr<services::Connection> connectionPtr{ connection.Emplace() };
    testing::StrictMock<services::ConnectionFactoryMock> connectionFactoryMock;
    testing::StrictMock<hal::SerialCommunicationMock> serialCommunicationMock;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    infra::Execute execute{ [this] {
        EXPECT_CALL(connectionFactoryMock, Listen(9000, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr)));
    } };
    services::SerialServer::WithBuffer<128> serialServer{ serialCommunicationMock, connectionFactoryMock, 9000 };
};

TEST_F(SerialServerTest, forward_data_from_serial_to_socket)
{
    serialCommunicationMock.dataReceived(std::vector<uint8_t>{ 'A', 'B', 'C' });
    ExecuteAllActions();

    EXPECT_EQ("ABC", connection->SentDataAsString());
}

TEST_F(SerialServerTest, forward_data_from_socket_to_serial)
{
    EXPECT_CALL(serialCommunicationMock, SendDataMock(std::vector<uint8_t>{ 'C', 'B', 'A' }));
    connection->SimulateDataReceived(std::vector<uint8_t>{ 'C', 'B', 'A' });
}
