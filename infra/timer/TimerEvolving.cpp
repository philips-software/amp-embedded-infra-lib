#include "infra/timer/TimerEvolving.hpp"

namespace infra
{
    TimerEvolving::TimerEvolving(uint32_t timerServiceId)
        : Timer(timerServiceId)
        , index(0)
    {}

    TimerEvolving::TimerEvolving(const infra::Function<infra::Optional<infra::Duration>(uint32_t index)>& evolve, const infra::Function<void()>& action, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(evolve, action);
    }

    void TimerEvolving::Start(const infra::Function<infra::Optional<infra::Duration>(uint32_t index)>& evolve, const infra::Function<void()>& action)
    {
        index = 0;
        this->evolve = evolve;

        triggerStart = Now() + Resolution();

        Evolve(action);
    }

    void TimerEvolving::ComputeNextTriggerTime()
    {
        Evolve(Action());
    }

    void TimerEvolving::Evolve(const infra::Function<void()>& action)
    {
        auto nextDuration = evolve(index);
        ++index;

        if (nextDuration)
        {
            triggerStart += *nextDuration;
            SetNextTriggerTime(triggerStart, action);
        }
        else
            Cancel();
    }
}
