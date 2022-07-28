#include "infra/event/EventDispatcher.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"

namespace infra
{
    ClockFixture::ClockFixture(uint32_t timerSericeId)
        : systemTimerService(timerSericeId)
    {}

    ClockFixture::~ClockFixture()
    {}

    void ClockFixture::ForwardTime(Duration amount)
    {
        ExecuteAllActions();
        TimePoint newSystemTime = systemTimerService.Now() + amount;

        do
        {
            TimePoint minimumTriggerTime = std::min(newSystemTime, systemTimerService.NextTrigger());

            systemTimerService.TimeProgressed(minimumTriggerTime - systemTimerService.Now());
            ExecuteAllActions();
        } while (systemTimerService.Now() != newSystemTime);
    }

    void ClockFixture::JumpForwardTime(Duration amount)
    {
        ExecuteAllActions();
        systemTimerService.TimeJumped(amount);
        ExecuteAllActions();
    }

    ClockFixture::TimeMatcherHelper ClockFixture::After(Duration duration) const
    {
        return TimeMatcherHelper(systemTimerService.Now() + duration);
    }

    std::string ClockFixture::TimeToString(TimePoint time)
    {
        std::ostringstream os;
        PrintTo(time, &os);
        return os.str();
    }

    ClockFixture::TimeMatcherHelper::TimeMatcherHelper(infra::TimePoint expectedCallTime)
        : expectedCallTime(expectedCallTime)
    {}
}
