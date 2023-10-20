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
    infra::VerifyingFunctionMock<void(infra::ConstByteRange)> clientReceiveCallback{ infra::MakeStringByteRange("hello") };

    client.ReceiveData(clientReceiveCallback);
    server.SendData(infra::MakeStringByteRange("hello"), infra::emptyFunction);

    ExecuteAllActions();
}

TEST_F(SerialCommunicationLoopbackTest, SendFromClientReceiveByServer)
{
    infra::VerifyingFunctionMock<void(infra::ConstByteRange)> serverReceiveCallback{ infra::MakeStringByteRange("world") };

    server.ReceiveData(serverReceiveCallback);
    client.SendData(infra::MakeStringByteRange("world"), infra::emptyFunction);

    ExecuteAllActions();
}

TEST_F(SerialCommunicationLoopbackTest, ActionOnCompletionAfterDataReceivedByOtherPeer)
{
    infra::MockCallback<void(infra::ConstByteRange)> dataReceivedMockCallback;
    infra::MockCallback<void()> actionOnCompletionMockCallback;

    server.ReceiveData([&dataReceivedMockCallback](infra::ConstByteRange data)
        {
            dataReceivedMockCallback.callback(data);
        });

    client.SendData(infra::MakeStringByteRange("Hello World!"), [&actionOnCompletionMockCallback]
        {
            actionOnCompletionMockCallback.callback();
        });

    testing::InSequence sequence;
    EXPECT_CALL(dataReceivedMockCallback, callback(infra::MakeStringByteRange("Hello World!")));
    EXPECT_CALL(actionOnCompletionMockCallback, callback());

    ExecuteAllActions();
}
