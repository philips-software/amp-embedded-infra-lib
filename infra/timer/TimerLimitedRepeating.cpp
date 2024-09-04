#include "infra/timer/TimerLimitedRepeating.hpp"

namespace infra
{
    TimerLimitedRepeating::TimerLimitedRepeating(int aHowMany, Duration duration, const infra::Function<void()>& aAction, uint32_t timerServiceId)
        : detail::TimerRepeating(timerServiceId)
    {
        Start(aHowMany, duration, aAction);
    }

    void TimerLimitedRepeating::Start(int aHowMany, Duration duration, const infra::Function<void()>& aAction)
    {
        howMany = aHowMany;
        if (howMany > 0)
            StartTimer(duration, aAction);
        else
            Cancel();
    }

    void TimerLimitedRepeating::ComputeNextTriggerTime()
    {
        --howMany;
        if (howMany > 0)
            detail::TimerRepeating::ComputeNextTriggerTime();
        else
            Cancel();
    }
}
