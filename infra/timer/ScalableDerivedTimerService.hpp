#ifndef INFRA_SCALABLE_DERIVED_TIMER_SERVICE_HPP
#define INFRA_SCALABLE_DERIVED_TIMER_SERVICE_HPP

#include "infra/timer/TimerService.hpp"

namespace infra
{
    class ScalableDerivedTimerService
        : public TimerService
    {
    public:
        ScalableDerivedTimerService(uint32_t id, TimerService& baseTimerService);

        // Implementation of TimerService
        virtual void NextTriggerChanged() override;
        virtual TimePoint Now() const override;
        virtual Duration Resolution() const override;

        void Shift(Duration shift);
        void Scale(uint32_t factor);
        Duration GetCurrentShift() const;

    private:
        TimePoint ScaleTime(TimePoint time) const;
        TimePoint UnscaleTime(TimePoint time) const;

    private:
        TimerService& baseTimerService;
        TimerSingleShot timer;
        TimePoint nextTrigger;

        Duration shift = Duration();
        uint32_t factor = 1;
        TimePoint startPointForFactor;
    };

}

#endif
