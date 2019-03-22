#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "infra/timer/TimerLimitedRepeating.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"

class TimerLimitedRepeatingTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(TimerLimitedRepeatingTest, ScheduleNTimesRepeatedly)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));

    infra::TimerLimitedRepeating timer(2, std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
}

TEST_F(TimerLimitedRepeatingTest, TimerDoesNotStayArmedWhenNoExtraAction)
{
    infra::TimerLimitedRepeating timer(5, std::chrono::seconds(1), []() {});

    ForwardTime(std::chrono::seconds(5));
    EXPECT_FALSE(timer.Armed());
}

TEST_F(TimerLimitedRepeatingTest, LimitedTimerExecutesNTimes)
{
    infra::MockCallback<void()> callback;

    infra::TimerLimitedRepeating timer(5, std::chrono::seconds(1), [&callback]() { callback.callback(); });

    EXPECT_CALL(callback, callback()).Times(5);
    ForwardTime(std::chrono::seconds(10));
}

TEST_F(TimerLimitedRepeatingTest, ResolutionIsTakenIntoAccount)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(3)));

    systemTimerService.SetResolution(std::chrono::seconds(1));

    infra::TimerLimitedRepeating timer(2, std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
}
