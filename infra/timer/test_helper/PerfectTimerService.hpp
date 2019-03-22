#ifndef INFRA_PERFECT_TIMER_SERVICE_HPP
#define INFRA_PERFECT_TIMER_SERVICE_HPP

#include "infra/timer/TimerService.hpp"

namespace infra
{
    class PerfectTimerService
        : public TimerService
    {
    public:
        PerfectTimerService(uint32_t id);

        virtual void NextTriggerChanged() override;
        virtual TimePoint Now() const override;
        virtual Duration Resolution() const override;

        void SetResolution(Duration resolution);
        void TimeProgressed(Duration amount);

    private:
        TimePoint systemTime = TimePoint();
        Duration resolution;

        uint32_t nextNotification;
        infra::TimePoint previousTrigger;
    };
}

#endif