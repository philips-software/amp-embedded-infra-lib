#ifndef INFRA_TIMER_ALTERNATING_HPP
#define INFRA_TIMER_ALTERNATING_HPP

#include "infra/timer/Timer.hpp"

// Like TimerRepeating, but alternates between two different actions, with two different intermediate durations
// Handy when blinking LEDs on and off

namespace infra
{
    class TimerAlternating
        : public Timer
    {
    public:
        explicit TimerAlternating(uint32_t timerServiceId = systemTimerServiceId);
        TimerAlternating(Duration duration1, const infra::Function<void()>& aAction1, Duration duration2, const infra::Function<void()>& aAction2, uint32_t timerServiceId = systemTimerServiceId);

        void Start(Duration duration1, const infra::Function<void()>& aAction1, Duration duration2, const infra::Function<void()>& aAction2);

    protected:
        virtual const infra::Function<void()>& Action() const override;
        virtual void ComputeNextTriggerTime() override;

    private:
        std::array<Duration, 2> durations = {};
        std::array<infra::Function<void()>, 2> scheduledTriggers;
        std::size_t index;

        TimePoint triggerStart;
    };
}

#endif
