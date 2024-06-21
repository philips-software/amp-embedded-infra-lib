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

    void TimerLimitedRepeating::Start(int aHowMany, Duration duration, const infra::Function<void()>& action)
    {
        if (aHowMany > 0)
        {
            howMany = aHowMany;
            triggerPeriod = duration;
            SetNextTriggerTime(Now() + triggerPeriod + Resolution(), action);
        }
        else
        {
            Cancel();
        }
    }

    void TimerLimitedRepeating::ComputeNextTriggerTime()
    {
        --howMany;
        if (howMany != 0)
        {
            TimePoint now = std::max(Now(), NextTrigger());
            Duration diff = (now - NextTrigger()) % triggerPeriod;

            SetNextTriggerTime(now - diff + triggerPeriod, Action());
        }
        else
        {
            Cancel();
        }
    }
}
