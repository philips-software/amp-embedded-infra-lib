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
        void NextTriggerChanged() override;
        TimePoint Now() const override;
        Duration Resolution() const override;

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
