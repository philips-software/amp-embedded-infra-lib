#include "infra/timer/TimerLimitedRepeatingWithClosingAction.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class TimerLimitedRepeatingWithClosingActionTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(TimerLimitedRepeatingWithClosingActionTest, TimerIsExecuted)
{
    infra::MockCallback<void()> callback;
    infra::MockCallback<void()> extra;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));
    EXPECT_CALL(extra, callback()).With(After(std::chrono::seconds(3)));

    infra::TimerLimitedRepeatingWithClosingAction timer(
        2, std::chrono::seconds(1), [&callback]()
        {
            callback.callback();
        },
        [&extra]()
        {
            extra.callback();
        });

    ForwardTime(std::chrono::seconds(5));
}

TEST_F(TimerLimitedRepeatingWithClosingActionTest, TimerStaysArmedBeforeExtraAction)
{
    infra::TimerLimitedRepeatingWithClosingAction timer(5, std::chrono::seconds(1), infra::emptyFunction, infra::emptyFunction);

    ForwardTime(std::chrono::seconds(5));
    EXPECT_TRUE(timer.Armed());
}

TEST_F(TimerLimitedRepeatingWithClosingActionTest, TimerIsNotArmedAfterExtraAction)
{
    infra::TimerLimitedRepeatingWithClosingAction timer(5, std::chrono::seconds(1), infra::emptyFunction, infra::emptyFunction);

    ForwardTime(std::chrono::seconds(6));
    EXPECT_FALSE(timer.Armed());
}

TEST_F(TimerLimitedRepeatingWithClosingActionTest, ResolutionIsTakenIntoAccount)
{
    systemTimerService.SetResolution(std::chrono::seconds(1));

    infra::MockCallback<void()> callback;
    infra::MockCallback<void()> extra;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(3)));
    EXPECT_CALL(extra, callback()).With(After(std::chrono::seconds(4)));

    infra::TimerLimitedRepeatingWithClosingAction timer(
        2, std::chrono::seconds(1), [&callback]()
        {
            callback.callback();
        },
        [&extra]()
        {
            extra.callback();
        });

    ForwardTime(std::chrono::seconds(5));
}

TEST_F(TimerLimitedRepeatingWithClosingActionTest, CancelAfterFirstCallback)
{
    infra::MockCallback<void()> callback;
    infra::MockCallback<void()> extra;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    infra::TimerLimitedRepeatingWithClosingAction timer(
        2, std::chrono::seconds(1), [&callback, &timer]()
        {
            callback.callback();
            timer.Cancel();
        },
        [&extra]()
        {
            extra.callback();
        });

    ForwardTime(std::chrono::seconds(5));
}

TEST_F(TimerLimitedRepeatingWithClosingActionTest, Restart0AfterFirstCallback)
{
    infra::MockCallback<void()> callback;
    infra::MockCallback<void()> extra;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    infra::TimerLimitedRepeatingWithClosingAction timer(
        2, std::chrono::seconds(1), [&callback, &timer]()
        {
            callback.callback();
            timer.Start(
                0, std::chrono::seconds(1), [&callback]()
                {
                    callback.callback();
                },
                []() {});
        },
        [&extra]()
        {
            extra.callback();
        });

    ForwardTime(std::chrono::seconds(5));
}
