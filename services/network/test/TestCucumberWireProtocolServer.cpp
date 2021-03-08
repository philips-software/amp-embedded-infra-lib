#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockHelpers.hpp"
#include "services/network/CucumberWireProtocolServer.hpp"
#include "services/network/test_doubles/ConnectionMock.hpp"
#include "services/network/test_doubles/ConnectionStub.hpp"
#include "gmock/gmock.h"

class CucumberWireProtocolpServerTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    CucumberWireProtocolpServerTest()
        : connectionPtr(infra::UnOwnedSharedPtr(connection))
        , execute([this]() { EXPECT_CALL(connectionFactoryMock, Listen(1234, testing::_, services::IPVersions::both)).WillOnce(testing::DoAll(infra::SaveRef<1>(&serverConnectionObserverFactory), testing::Return(nullptr))); })
        , cucumberServer(connectionFactoryMock, 1234)
    {}

    ~CucumberWireProtocolpServerTest()
    {
        cucumberServer.Stop(infra::emptyFunction);
    }

    testing::StrictMock<services::ConnectionStub> connection;
    infra::SharedPtr<services::ConnectionStub> connectionPtr;
    testing::StrictMock<services::ConnectionFactoryMock> connectionFactoryMock;
    services::ServerConnectionObserverFactory* serverConnectionObserverFactory;
    infra::Execute execute;
    services::CucumberWireProtocolServer cucumberServer;
};

TEST_F(CucumberWireProtocolpServerTest, accept_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv4AddressLocalHost());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}

TEST_F(CucumberWireProtocolpServerTest, accept_ipv6_connection)
{
    connectionFactoryMock.NewConnection(*serverConnectionObserverFactory, connection, services::IPv6AddressLocalHost());
    EXPECT_CALL(connection, AbortAndDestroyMock());
}
