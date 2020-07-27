#include "infra/timer/DerivedTimerService.hpp"

namespace infra
{
    DerivedTimerService::DerivedTimerService(uint32_t id, TimerService& baseTimerService)
        : TimerService(id)
        , baseTimerService(baseTimerService)
        , timer(baseTimerService.Id())
    {}

    void DerivedTimerService::NextTriggerChanged()
    {
        nextTrigger = NextTrigger() - shift;

        if (NextTrigger() != TimePoint::max())
            timer.Start(nextTrigger, [this]() { Progressed(nextTrigger + shift); });
        else
            timer.Cancel();
    }

    TimePoint DerivedTimerService::Now() const
    {
        return baseTimerService.Now() + shift;
    }

    Duration DerivedTimerService::Resolution() const
    {
        return baseTimerService.Resolution();
    }

    void DerivedTimerService::Shift(Duration shift)
    {
        auto oldTime = Now();
        this->shift = shift;
        Jumped(oldTime, Now());
    }

    Duration DerivedTimerService::GetCurrentShift() const
    {
        return shift;
    }
}
