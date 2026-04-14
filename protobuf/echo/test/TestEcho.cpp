#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>

class EchoTest
    : public testing::Test
{
public:
    testing::StrictMock<services::EchoMock> echo;
    services::ServiceStubProxy serviceProxy{ echo };
};

TEST_F(EchoTest, request_send_while_already_awaiting_grant_aborts)
{
    EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));
    serviceProxy.RequestSend([]() {});

    EXPECT_DEATH(
        {
            serviceProxy.RequestSend([]() {});
        },
        "");

    // Cancel to not trigger mock
    EXPECT_CALL(echo, CancelRequestSend(testing::Ref(serviceProxy)));
    serviceProxy.CancelRequestSend();
}

TEST_F(EchoTest, cancel_clears_pending_grant_and_notifies_echo)
{
    EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));
    serviceProxy.RequestSend([]() {});

    EXPECT_CALL(echo, CancelRequestSend(testing::Ref(serviceProxy)));
    serviceProxy.CancelRequestSend();
}

TEST_F(EchoTest, cancel_clears_requested_size)
{
    EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));
    serviceProxy.RequestSend([]() {});
    EXPECT_NE(serviceProxy.CurrentRequestedSize(), 0);

    EXPECT_CALL(echo, CancelRequestSend(testing::Ref(serviceProxy)));
    serviceProxy.CancelRequestSend();
    EXPECT_EQ(serviceProxy.CurrentRequestedSize(), 0);
}

TEST_F(EchoTest, cancel_without_pending_request_is_a_noop)
{
    serviceProxy.CancelRequestSend(); // no expectations set — must not call echo at all
}

TEST_F(EchoTest, cancel_and_retry_request_send_succeeds)
{
    // First request
    EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));
    serviceProxy.RequestSend([]() {});

    // Cancel it
    EXPECT_CALL(echo, CancelRequestSend(testing::Ref(serviceProxy)));
    serviceProxy.CancelRequestSend();

    // Second request on the same proxy must not assert
    EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));
    bool granted = false;
    serviceProxy.RequestSend([&granted]()
        {
            granted = true;
        });
    EXPECT_FALSE(granted);

    // Grant it to verify the new callback fires
    serviceProxy.GrantSend();
    EXPECT_TRUE(granted);
}
