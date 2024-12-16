#ifndef INFRA_TIMER_LIMITED_REPEATING_HPP
#define INFRA_TIMER_LIMITED_REPEATING_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/Function.hpp"
#include <cstdint>

namespace infra
{
    class TimerLimitedRepeating
        : public detail::TimerRepeating
    {
    public:
        using detail::TimerRepeating::TimerRepeating;
        TimerLimitedRepeating(uint32_t aHowMany, Duration duration, const infra::Function<void()>& action, uint32_t timerServiceId = systemTimerServiceId);
        TimerLimitedRepeating(uint32_t aHowMany, Duration duration, const infra::Function<void()>& action, TriggerImmediately, uint32_t timerServiceId = systemTimerServiceId);

        void Start(uint32_t aHowMany, Duration duration, const infra::Function<void()>& action);
        void Start(uint32_t aHowMany, Duration duration, const infra::Function<void()>& action, TriggerImmediately);

    protected:
        void ComputeNextTriggerTime() override;

    private:
        uint32_t howMany{ 0 };
    };
}

#endif
