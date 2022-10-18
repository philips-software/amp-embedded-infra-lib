#include "infra/timer/TimerLimitedRepeatingWithClosingAction.hpp"

namespace infra
{
    TimerLimitedRepeatingWithClosingAction::TimerLimitedRepeatingWithClosingAction(uint32_t timerServiceId)
        : Timer(timerServiceId)
    {}

    TimerLimitedRepeatingWithClosingAction::TimerLimitedRepeatingWithClosingAction(int aHowMany, Duration duration, const infra::Function<void()>& aAction, const infra::Function<void()>& aClosingAction, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(aHowMany, duration, aAction, aClosingAction);
    }

    void TimerLimitedRepeatingWithClosingAction::Start(int aHowMany, Duration duration, const infra::Function<void()>& aAction, const infra::Function<void()>& aClosingAction)
    {
        closingAction = aClosingAction;
        action = aAction;

        triggerStart = Now() + Resolution();
        triggerPeriod = duration;
        howMany = aHowMany;

        ComputeNextTriggerTime();
    }

    const infra::Function<void()>& TimerLimitedRepeatingWithClosingAction::Action() const
    {
        if (howMany >= 0)
            return action;
        else
            return closingAction;
    }

    void TimerLimitedRepeatingWithClosingAction::ComputeNextTriggerTime()
    {
        if (howMany >= 0)
        {
            --howMany;

            TimePoint now = Now();

            Duration diff = now - triggerStart;
            if (diff < Duration())
                now += triggerPeriod;
            diff %= triggerPeriod;
            SetNextTriggerTime(now - diff + triggerPeriod, infra::emptyFunction);
        }
        else
            Cancel();
    }
}
