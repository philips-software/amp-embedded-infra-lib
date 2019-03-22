#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "infra/event/EventDispatcher.hpp"
#include "infra/timer/DerivedTimerService.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"

class DerivedTimerServiceTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    DerivedTimerServiceTest()
        : scaleId(reinterpret_cast<std::ptrdiff_t>(&scaleId))
        , scalableTimerService(scaleId, systemTimerService)
    {}

    std::ptrdiff_t scaleId;
    infra::DerivedTimerService scalableTimerService;
};

TEST_F(DerivedTimerServiceTest, ShiftScalableTimerService)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).With(After(std::chrono::seconds(2)));

    infra::TimerSingleShot timer(std::chrono::seconds(3), [&callback]() { callback.callback(); }, scaleId);
    EXPECT_EQ(std::chrono::seconds(0), scalableTimerService.GetCurrentShift());
    scalableTimerService.Shift(std::chrono::seconds(2));
    EXPECT_EQ(std::chrono::seconds(2), scalableTimerService.GetCurrentShift());
    scalableTimerService.Shift(std::chrono::seconds(1));
    EXPECT_EQ(std::chrono::seconds(1), scalableTimerService.GetCurrentShift());

    ForwardTime(std::chrono::seconds(5));
}
