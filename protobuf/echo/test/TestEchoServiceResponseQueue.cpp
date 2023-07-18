#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "protobuf/echo/Echo.hpp"
#include "protobuf/echo/test_doubles/EchoMock.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace services
{
    class EchoServiceResponseQueueTest
        : public testing::Test
    {
    public:
        testing::StrictMock<EchoMock> echo;
        testing::StrictMock<ServiceProxyResponseQueue<ServiceStubProxy>>::WithStorage<2> serviceProxy{ echo };
    };

    TEST_F(EchoServiceResponseQueueTest, RequestSendWhileEmptyImmediatelyForwardsRequestToEcho)
    {
        EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)));

        serviceProxy.RequestSend(infra::emptyFunction);
    }

    TEST_F(EchoServiceResponseQueueTest, RequestsNextRequestAfterGrantingAccess)
    {
        infra::VerifyingFunctionMock<void()> request1{};
        infra::VerifyingFunctionMock<void()> request2{};

        EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy))).Times(2);

        serviceProxy.RequestSend(request1, 128);
        serviceProxy.RequestSend(request2, 50);

        EXPECT_THAT(serviceProxy.CurrentRequestedSize(), testing::Eq(128));
        serviceProxy.GrantSend();

        EXPECT_THAT(serviceProxy.CurrentRequestedSize(), testing::Eq(50));
        serviceProxy.GrantSend();
    }

    TEST_F(EchoServiceResponseQueueTest, DiscardsRequestWhenQueueFull)
    {
        infra::VerifyingFunctionMock<void()> request1{};
        infra::VerifyingFunctionMock<void()> request2{};
        infra::VerifyingFunctionMock<void()> request4{};

        EXPECT_CALL(echo, RequestSend(testing::Ref(serviceProxy)))
            .Times(3);

        serviceProxy.RequestSend(request1);
        serviceProxy.RequestSend(request2);

        serviceProxy.RequestSend([]
            {
                FAIL();
            });

        serviceProxy.GrantSend();
        serviceProxy.GrantSend();

        serviceProxy.RequestSend(request4);
        serviceProxy.GrantSend();
    }
}
