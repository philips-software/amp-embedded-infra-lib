#include "gtest/gtest.h"
#include "infra/timer/Timer.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"

class TimerTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(TimerTest, SingleShotTimerTriggersOnceAfterDuration)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    infra::TimerSingleShot timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
};

TEST_F(TimerTest, SingleShotTimerTriggersOnceAfterTime)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    infra::TimerSingleShot timer(infra::Now() + std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
};

TEST_F(TimerTest, SingleShotTimerIsCancellable)
{
    infra::MockCallback<void(infra::TimePoint)> callback;

    infra::TimerSingleShot timer(std::chrono::seconds(1), [this, &callback]() { callback.callback(systemTimerService.Now()); });

    timer.Cancel();

    ForwardTime(std::chrono::seconds(5));
};

TEST_F(TimerTest, SingleShotTimerTakesResolutionIntoAccount)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));

    systemTimerService.SetResolution(std::chrono::seconds(1));
    infra::TimerSingleShot timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
};

TEST_F(TimerTest, TwoTimersTriggerInOrder)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));

    infra::TimerSingleShot timer1(std::chrono::seconds(1), [&callback]() { callback.callback(); });
    infra::TimerSingleShot timer2(std::chrono::seconds(2), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(5));
};

TEST_F(TimerTest, RepeatingTimerTriggersRepeatedly)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(3)));

    infra::TimerRepeating timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(3));
};

TEST_F(TimerTest, RepeatingTimerTriggersImmediately)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(0)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    infra::TimerRepeating timer(
        std::chrono::seconds(1), [&callback]() { callback.callback(); }, infra::triggerImmediately);

    ForwardTime(std::chrono::seconds(1));
}

TEST_F(TimerTest, RepeatingTimerTriggersRepeatedlyAfterReset)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(3)));

    infra::TimerRepeating timer;
    timer.Start(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(3));
};

TEST_F(TimerTest, RepeatingTimerTriggersImmediatelyAfterReset)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(0)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    infra::TimerRepeating timer;
    timer.Start(
        std::chrono::seconds(1), [&callback]() { callback.callback(); }, infra::triggerImmediately);

    ForwardTime(std::chrono::seconds(1));
}

TEST_F(TimerTest, RepeatingTimerIsCancellable)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    infra::TimerRepeating timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(1));
    timer.Cancel();
    ForwardTime(std::chrono::seconds(1));
};

TEST_F(TimerTest, RepeatingTimerTakesResolutionIntoAccount)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(1050)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(2050)));
    systemTimerService.SetResolution(std::chrono::milliseconds(50));

    infra::TimerRepeating timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });

    ForwardTime(std::chrono::seconds(3));
};

TEST_F(TimerTest, TestTimerNotArmedAfterConstruction)
{
    infra::TimerSingleShot timer;

    EXPECT_FALSE(timer.Armed());
}

TEST_F(TimerTest, TestTimerArmed)
{
    infra::TimerSingleShot timer(std::chrono::seconds(1), []() {});

    EXPECT_TRUE(timer.Armed());
}

TEST_F(TimerTest, TestTimerNotArmedAfterTrigger)
{
    infra::TimerSingleShot timer(std::chrono::seconds(1), []() {});

    ForwardTime(std::chrono::seconds(5));
    EXPECT_FALSE(timer.Armed());
}

TEST_F(TimerTest, TestTimerStaysArmedAfterRepeatedTrigger)
{
    infra::TimerRepeating timer(std::chrono::seconds(1), []() {});

    ForwardTime(std::chrono::seconds(5));
    EXPECT_TRUE(timer.Armed());
}

TEST_F(TimerTest, TestTimerCancel)
{
    testing::StrictMock<infra::MockCallback<void()>> callback;

    infra::TimerSingleShot timer(std::chrono::seconds(1), [&callback]() { callback.callback(); });
    timer.Cancel();

    ForwardTime(std::chrono::seconds(5));
}

TEST_F(TimerTest, TestTimerIsNotArmedAfterCancel)
{
    infra::TimerSingleShot timer(std::chrono::seconds(1), []() {});
    timer.Cancel();

    EXPECT_FALSE(timer.Armed());
}
