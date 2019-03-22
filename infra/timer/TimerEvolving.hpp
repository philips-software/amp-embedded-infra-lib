#ifndef INFRA_TIMER_EVOLVING_HPP
#define INFRA_TIMER_EVOLVING_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/Optional.hpp"

namespace infra
{
    class TimerEvolving
        : public Timer
    {
    public:
        explicit TimerEvolving(uint32_t timerServiceId = systemTimerServiceId);
        TimerEvolving(const infra::Function<infra::Optional<infra::Duration>(uint32_t index)>& evolve, const infra::Function<void()>& action, uint32_t timerServiceId = systemTimerServiceId);

        void Start(const infra::Function<infra::Optional<infra::Duration>(uint32_t index)>& evolve, const infra::Function<void()>& action);

    protected:
        virtual void ComputeNextTriggerTime() override;

    private:
        void Evolve(const infra::Function<void()>& action);

    private:
        infra::Function<infra::Optional<infra::Duration>(uint32_t index)> evolve;
        std::size_t index = 0;
        infra::TimePoint triggerStart;
    };
}

#endif
