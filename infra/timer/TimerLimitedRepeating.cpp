#include "infra/timer/TimerLimitedRepeating.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/Function.hpp"
#include <cstdint>

namespace infra
{
    TimerLimitedRepeating::TimerLimitedRepeating(uint32_t aHowMany, Duration duration, const infra::Function<void()>& action, uint32_t timerServiceId)
        : detail::TimerRepeating(timerServiceId)
    {
        Start(aHowMany, duration, action);
    }

    TimerLimitedRepeating::TimerLimitedRepeating(uint32_t aHowMany, Duration duration, const infra::Function<void()>& action, TriggerImmediately, uint32_t timerServiceId)
        : detail::TimerRepeating(timerServiceId)
    {
        Start(aHowMany, duration, action, triggerImmediately);
    }

    void TimerLimitedRepeating::Start(uint32_t aHowMany, Duration duration, const infra::Function<void()>& aAction)
    {
        howMany = aHowMany;
        if (howMany > 0)
            StartTimer(duration, aAction);
        else
            Cancel();
    }

    void TimerLimitedRepeating::Start(uint32_t aHowMany, Duration duration, const infra::Function<void()>& action, TriggerImmediately)
    {
        const auto atLeastOnce = aHowMany > 0;
        const auto howManyRemaining = atLeastOnce ? aHowMany - 1 : 0;

        Start(howManyRemaining, duration, action);

        if (atLeastOnce)
            action();
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
