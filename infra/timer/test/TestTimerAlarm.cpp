#include "infra/timer/TimerAlarm.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gtest/gtest.h"

class TimerAlarmTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(TimerAlarmTest, TimerAlarmTriggersOnceAfterDuration)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    infra::TimerAlarm timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
};

TEST_F(TimerAlarmTest, TimerAlarmTriggersAgainAfterADay)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1) + std::chrono::hours(24)));

    infra::TimerAlarm timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5) + std::chrono::hours(24));
};

TEST_F(TimerAlarmTest, TimerAlarmTriggersAgainWhenTimeJumpsBack)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1))).Times(2);

    infra::TimerAlarm timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
    JumpForwardTime(-std::chrono::hours(1));
    ForwardTime(std::chrono::hours(1));
};

TEST_F(TimerAlarmTest, TimerAlarmTriggersAgainNextDayEvenWhenTimeJumpedInBetween)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1) + std::chrono::hours(24)));

    infra::TimerAlarm timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
    JumpForwardTime(std::chrono::hours(1));
    ForwardTime(std::chrono::hours(23));
};

TEST_F(TimerAlarmTest, TimerAlarmTriggersAgainTodayEvenWhenTimeJumpedInBetween)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::hours(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::hours(1) + std::chrono::hours(24)));

    infra::TimerAlarm timer(std::chrono::hours(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::hours(24) + std::chrono::minutes(1));
    JumpForwardTime(std::chrono::minutes(1));
    ForwardTime(std::chrono::hours(1));
};

TEST_F(TimerAlarmTest, TimerAlarmTriggersDayLaterTimeJumpedOverTriggerMoment)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1) + std::chrono::hours(24)));

    infra::TimerAlarm timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    JumpForwardTime(std::chrono::hours(1));
    ForwardTime(std::chrono::hours(24));
};
