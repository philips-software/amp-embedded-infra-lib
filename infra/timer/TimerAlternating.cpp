#include "infra/timer/TimerAlternating.hpp"

namespace infra
{
    TimerAlternating::TimerAlternating(uint32_t timerServiceId)
        : Timer(timerServiceId)
    {}

    TimerAlternating::TimerAlternating(Duration duration1, const infra::Function<void()>& aAction1, Duration duration2, const infra::Function<void()>& aAction2, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(duration1, aAction1, duration2, aAction2);
    }

    void TimerAlternating::Start(Duration duration1, const infra::Function<void()>& aAction1, Duration duration2, const infra::Function<void()>& aAction2)
    {
        index = 1;
        triggerStart = Now() + Resolution();

        durations[0] = duration1;
        durations[1] = duration2;
        scheduledTriggers[0] = aAction1;
        scheduledTriggers[1] = aAction2;

        ComputeNextTriggerTime();
    }

    const infra::Function<void()>& TimerAlternating::Action() const
    {
        return scheduledTriggers[index];
    }

    void TimerAlternating::ComputeNextTriggerTime()
    {
        index = (index + 1) & 1;

        triggerStart += durations[index];
        SetNextTriggerTime(triggerStart, infra::emptyFunction); // Dummy function, this is overruled in Action()
    }
}
