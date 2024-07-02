#ifndef INFRA_TIMER_LIMITED_REPEATING_HPP
#define INFRA_TIMER_LIMITED_REPEATING_HPP

#include "infra/timer/Timer.hpp"

namespace infra
{
    class TimerLimitedRepeating
        : public details::TimerRepeating
    {
    public:
        using details::TimerRepeating::TimerRepeating;
        TimerLimitedRepeating(int aHowMany, Duration duration, const infra::Function<void()>& action, uint32_t timerServiceId = systemTimerServiceId);

        void Start(int aHowMany, Duration duration, const infra::Function<void()>& action);

    protected:
        void ComputeNextTriggerTime() override;

    private:
        int howMany{ 0 };
    };
}

#endif
