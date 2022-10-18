#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/timer/TickOnInterruptTimerService.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class BaseTimerServiceTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    BaseTimerServiceTest()
        : timerService(infra::systemTimerServiceId, std::chrono::seconds(1))
    {}

    infra::TickOnInterruptTimerService timerService;
};

TEST_F(BaseTimerServiceTest, set_resolution)
{
    infra::VerifyingFunctionMock<void()> verify;
    infra::TimerSingleShot timer(std::chrono::seconds(1), verify);

    timerService.SetResolution(std::chrono::seconds(2));
    ASSERT_EQ(infra::TimePoint(), timerService.Now());
    timerService.SystemTickInterrupt();
    ExecuteAllActions();
    EXPECT_EQ(infra::TimePoint() + std::chrono::seconds(2), timerService.Now());
}
