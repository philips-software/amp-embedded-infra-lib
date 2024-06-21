#include "infra/timer/TimerLimitedRepeatingWithClosingAction.hpp"

namespace infra
{
    TimerLimitedRepeatingWithClosingAction::TimerLimitedRepeatingWithClosingAction(int aHowMany, Duration duration, const infra::Function<void()>& aAction, const infra::Function<void()>& aClosingAction, uint32_t timerServiceId)
        : details::TimerRepeating(timerServiceId)
    {
        Start(aHowMany, duration, aAction, aClosingAction);
    }

    void TimerLimitedRepeatingWithClosingAction::Start(int aHowMany, Duration duration, const infra::Function<void()>& aAction, const infra::Function<void()>& aClosingAction)
    {
        howMany = aHowMany;
        closingAction = aClosingAction;
        if (howMany > 0)
        {
            StartTimer(duration, aAction);
        }
        else
        {
            Cancel();
            closingAction();
        }
    }

    const infra::Function<void()>& TimerLimitedRepeatingWithClosingAction::Action() const
    {
        return howMany > 0 ? Timer::Action() : closingAction;
    }

    void TimerLimitedRepeatingWithClosingAction::ComputeNextTriggerTime()
    {
        --howMany;
        if (howMany >= 0)
            details::TimerRepeating::ComputeNextTriggerTime();
        else
            Cancel();
    }
}
