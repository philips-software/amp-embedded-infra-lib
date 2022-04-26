#include "gtest/gtest.h"
#include "infra/timer/TimerAlternating.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"

class TimerAlternatingTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(TimerAlternatingTest, AlternatingTimer_TriggersCallbackInSuccession)
{
    infra::Duration delta1 = std::chrono::seconds(1);
    infra::Duration delta2 = std::chrono::seconds(2);

    infra::MockCallback<void()> callback1;
    EXPECT_CALL(callback1, callback()).With(After(delta1));
    EXPECT_CALL(callback1, callback()).With(After(2 * delta1 + delta2));

    infra::MockCallback<void()> callback2;
    EXPECT_CALL(callback2, callback()).With(After(delta1 + delta2));
    EXPECT_CALL(callback2, callback()).With(After(2 * delta1 + 2 * delta2));

    infra::TimerAlternating timer(
        delta1, [&callback1]() { callback1.callback(); }, delta2, [&callback2]() { callback2.callback(); });

    ForwardTime(std::chrono::seconds(6));
};

TEST_F(TimerAlternatingTest, ResolutionIsTakenIntoAccount)
{
    systemTimerService.SetResolution(std::chrono::seconds(1));

    infra::MockCallback<void()> callback1;
    EXPECT_CALL(callback1, callback()).With(After(std::chrono::seconds(2)));
    infra::MockCallback<void()> callback2;
    EXPECT_CALL(callback2, callback()).With(After(std::chrono::seconds(3)));

    infra::TimerAlternating timer(
        std::chrono::seconds(1), [&callback1]() { callback1.callback(); }, std::chrono::seconds(1), [&callback2]() { callback2.callback(); });

    ForwardTime(std::chrono::seconds(3));
}
