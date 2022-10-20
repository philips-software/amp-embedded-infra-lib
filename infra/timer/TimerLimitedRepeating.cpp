#include "infra/timer/TimerLimitedRepeating.hpp"

namespace infra
{
    TimerLimitedRepeating::TimerLimitedRepeating(uint32_t timerServiceId)
        : Timer(timerServiceId)
    {}

    TimerLimitedRepeating::TimerLimitedRepeating(int aHowMany, Duration duration, const infra::Function<void()>& aAction, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(aHowMany, duration, aAction);
    }

    void TimerLimitedRepeating::Start(int aHowMany, Duration duration, const infra::Function<void()>& aAction)
    {
        action = aAction;

        triggerStart = Now() + Resolution();
        triggerPeriod = duration;
        howMany = aHowMany;

        ComputeNextTriggerTime();
    }

    void TimerLimitedRepeating::ComputeNextTriggerTime()
    {
        if (howMany != 0)
        {
            --howMany;

            TimePoint now = Now();

            Duration diff = now - triggerStart;
            if (diff < Duration())
                now += triggerPeriod;
            diff %= triggerPeriod;
            SetNextTriggerTime(now - diff + triggerPeriod, action);
        }
        else
            Cancel();
    }
}
