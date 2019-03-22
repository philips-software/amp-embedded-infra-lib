#ifndef INFRA_DERIVED_TIMER_SERVICE_HPP
#define INFRA_DERIVED_TIMER_SERVICE_HPP

#include "infra/timer/TimerService.hpp"

namespace infra
{
    class DerivedTimerService
        : public TimerService
    {
    public:
        DerivedTimerService(uint32_t id, TimerService& baseTimerService);

        // Implementation of TimerService
        virtual void NextTriggerChanged() override;
        virtual TimePoint Now() const override;
        virtual Duration Resolution() const override;

        void Shift(Duration shift);
        Duration GetCurrentShift() const;

    private:
        TimerService& baseTimerService;
        TimerSingleShot timer;
        TimePoint nextTrigger;

        Duration shift = Duration();
    };

}

#endif
