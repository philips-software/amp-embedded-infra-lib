#ifndef INFRA_TIMER_LIMITED_REPEATING_WITH_CLOSING_ACTION_HPP
#define INFRA_TIMER_LIMITED_REPEATING_WITH_CLOSING_ACTION_HPP

#include "infra/timer/Timer.hpp"

// Trigers a number of times, then triggers another action
// Useful in a protocol where after a number of retries failure has to be indicated

namespace infra
{
    class TimerLimitedRepeatingWithClosingAction
        : public Timer
    {
    public:
        explicit TimerLimitedRepeatingWithClosingAction(uint32_t timerServiceId = systemTimerServiceId);
        TimerLimitedRepeatingWithClosingAction(int aHowMany, Duration duration, const infra::Function<void()>& action, const infra::Function<void()>& aClosingAction, uint32_t timerServiceId = systemTimerServiceId);

        void Start(int aHowMany, Duration duration, const infra::Function<void()>& action, const infra::Function<void()>& aClosingAction);

    protected:
        virtual const infra::Function<void()>& Action() const override;
        virtual void ComputeNextTriggerTime() override;

    private:
        TimePoint triggerStart;
        Duration triggerPeriod;
        int howMany{ 0 };
        infra::Function<void()> action;
        infra::Function<void()> closingAction;
    };
}

#endif
