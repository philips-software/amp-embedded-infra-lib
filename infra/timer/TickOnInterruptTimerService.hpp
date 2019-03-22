#ifndef INFRA_TICK_ON_INTERRUPT_TIMER_SERVICE_HPP
#define INFRA_TICK_ON_INTERRUPT_TIMER_SERVICE_HPP

#include "infra/timer/TimerService.hpp"
#include <atomic>

namespace infra
{
    class TickOnInterruptTimerService
        : public TimerService
    {
    public:
        TickOnInterruptTimerService(uint32_t id, Duration resolution);

        virtual void NextTriggerChanged() override;
        virtual TimePoint Now() const override;
        virtual Duration Resolution() const override;

        void SetResolution(Duration resolution);

        void TimeProgressed(Duration amount);
        void SystemTickInterrupt();

    private:
        void ProcessTicks();

    private:
        TimePoint systemTime = TimePoint();
        Duration resolution;

        std::atomic<uint32_t> ticksNextNotification;
        std::atomic<uint32_t> ticksProgressed;
        std::atomic_bool notificationScheduled;
    };
}

#endif
