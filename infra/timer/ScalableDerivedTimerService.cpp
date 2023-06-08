#include "infra/timer/ScalableDerivedTimerService.hpp"

namespace infra
{
    ScalableDerivedTimerService::ScalableDerivedTimerService(uint32_t id, TimerService& baseTimerService)
        : TimerService(id)
        , baseTimerService(baseTimerService)
        , timer(baseTimerService.Id())
    {}

    void ScalableDerivedTimerService::NextTriggerChanged()
    {
        nextTrigger = UnscaleTime(NextTrigger());

        if (NextTrigger() != TimePoint::max())
            timer.Start(nextTrigger, [this]()
                {
                    Progressed(ScaleTime(nextTrigger));
                });
        else
            timer.Cancel();
    }

    TimePoint ScalableDerivedTimerService::Now() const
    {
        return ScaleTime(baseTimerService.Now());
    }

    Duration ScalableDerivedTimerService::Resolution() const
    {
        return baseTimerService.Resolution() * factor;
    }

    void ScalableDerivedTimerService::Shift(Duration shift)
    {
        this->shift = shift;
        NextTriggerChanged();
    }

    void ScalableDerivedTimerService::Scale(uint32_t factor)
    {
        startPointForFactor = Now();
        this->factor = factor;
        shift += startPointForFactor - Now();
        NextTriggerChanged();
    }

    Duration ScalableDerivedTimerService::GetCurrentShift() const
    {
        return shift;
    }

    TimePoint ScalableDerivedTimerService::ScaleTime(TimePoint time) const
    {
        return startPointForFactor + (time - startPointForFactor + shift) * factor;
    }

    TimePoint ScalableDerivedTimerService::UnscaleTime(TimePoint time) const
    {
        return (time - startPointForFactor) / factor - shift + startPointForFactor;
    }
}
