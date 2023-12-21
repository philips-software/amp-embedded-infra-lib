#ifndef INFRA_PERFECT_TIMER_SERVICE_HPP
#define INFRA_PERFECT_TIMER_SERVICE_HPP

#include "infra/timer/TimerService.hpp"

namespace infra
{
    class PerfectTimerService
        : public TimerService
    {
    public:
        explicit PerfectTimerService(uint32_t id);

        void NextTriggerChanged() override;
        TimePoint Now() const override;
        Duration Resolution() const override;

        void SetResolution(Duration resolution);
        void TimeProgressed(Duration amount);
        void TimeJumped(Duration amount);

    private:
        TimePoint systemTime;
        Duration resolution{ std::chrono::milliseconds(0) };
        uint32_t nextNotification;
        infra::TimePoint previousTrigger;
    };
}

#endif
