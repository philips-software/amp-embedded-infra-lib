#include "infra/event/EventDispatcher.hpp"
#include "infra/timer/ScalableDerivedTimerService.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class ScalableDerivedTimerServiceTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    ScalableDerivedTimerServiceTest()
        : scaleId(reinterpret_cast<std::ptrdiff_t>(&scaleId))
        , scalableTimerService(scaleId, systemTimerService)
    {}

    std::ptrdiff_t scaleId;
    infra::ScalableDerivedTimerService scalableTimerService;
};

TEST_F(ScalableDerivedTimerServiceTest, ShiftScalableTimerService)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));

    infra::TimerSingleShot timer(
        std::chrono::seconds(3), [&callback]()
        {
            callback.callback();
        },
        scaleId);
    scalableTimerService.Shift(std::chrono::seconds(1));

    ForwardTime(std::chrono::seconds(5));
}

TEST_F(ScalableDerivedTimerServiceTest, ScaleScalableTimerService)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(3)));

    scalableTimerService.Scale(2);
    infra::TimerRepeating timer(
        std::chrono::seconds(2), [&callback]()
        {
            callback.callback();
        },
        scaleId);

    ForwardTime(std::chrono::seconds(3));
}

TEST_F(ScalableDerivedTimerServiceTest, ScaleAndShiftScalableTimerService)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    scalableTimerService.Scale(2);
    infra::TimerSingleShot timer(
        std::chrono::seconds(4), [&callback]()
        {
            callback.callback();
        },
        scaleId);
    scalableTimerService.Shift(std::chrono::seconds(1));

    ForwardTime(std::chrono::seconds(3));
}

TEST_F(ScalableDerivedTimerServiceTest, ScaleAndShiftScalableTimerServiceAfterSomeTimePassed)
{
    ForwardTime(std::chrono::seconds(10));

    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));

    scalableTimerService.Scale(2);
    infra::TimerSingleShot timer(
        std::chrono::seconds(4), [&callback]()
        {
            callback.callback();
        },
        scaleId);
    scalableTimerService.Shift(std::chrono::seconds(1));

    ForwardTime(std::chrono::seconds(3));
}

TEST_F(ScalableDerivedTimerServiceTest, ScaleAndScaleBackAfterSomeTimePassed)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(1)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(3)));

    scalableTimerService.Scale(2);
    infra::TimerRepeating timer(
        std::chrono::seconds(2), [&callback]()
        {
            callback.callback();
        },
        scaleId);
    ForwardTime(std::chrono::seconds(1));
    scalableTimerService.Scale(1);

    ForwardTime(std::chrono::seconds(2));
}

TEST_F(ScalableDerivedTimerServiceTest, GetCurrentShiftValue)
{
    scalableTimerService.Shift(std::chrono::seconds(3));
    EXPECT_EQ(std::chrono::seconds(3), scalableTimerService.GetCurrentShift());
}
