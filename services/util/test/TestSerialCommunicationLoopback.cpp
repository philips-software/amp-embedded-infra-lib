#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/SerialCommunicationLoopback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SerialCommunicationLoopbackTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    services::SerialCommunicationLoopback SerialCommunicationLoopback;
    services::SerialCommunicationLoopbackPeer& server{ SerialCommunicationLoopback.Server() };
    services::SerialCommunicationLoopbackPeer& client{ SerialCommunicationLoopback.Client() };
};

TEST_F(SerialCommunicationLoopbackTest, SendFromServerReceiveByClient)
{
    infra::ConstByteRange data = infra::MakeStringByteRange("hello");
    infra::VerifyingFunctionMock<void(infra::ConstByteRange)> clientReceiveCallback{ data };

    client.ReceiveData(clientReceiveCallback);
    server.SendData(data, infra::emptyFunction);

    ExecuteAllActions();
}

TEST_F(SerialCommunicationLoopbackTest, SendFromClientReceiveByServer)
{
    infra::ConstByteRange data = infra::MakeStringByteRange("world");
    infra::VerifyingFunctionMock<void(infra::ConstByteRange)> serverReceiveCallback{ data };

    server.ReceiveData(serverReceiveCallback);
    client.SendData(data, infra::emptyFunction);

    ExecuteAllActions();
}

TEST_F(SerialCommunicationLoopbackTest, ActionOnCompletionAfterDataReceivedByOtherPeer)
{
    infra::ConstByteRange data = infra::MakeStringByteRange("Hello World!");
    infra::MockCallback<void(infra::ConstByteRange)> dataReceivedMockCallback;
    infra::MockCallback<void()> actionOnCompletionMockCallback;

    server.ReceiveData([&dataReceivedMockCallback](infra::ConstByteRange data)
        {
            dataReceivedMockCallback.callback(data);
        });

    client.SendData(data, [&actionOnCompletionMockCallback]
        {
            actionOnCompletionMockCallback.callback();
        });

    testing::InSequence sequence;
    EXPECT_CALL(dataReceivedMockCallback, callback(data));
    EXPECT_CALL(actionOnCompletionMockCallback, callback());

    ExecuteAllActions();
}
