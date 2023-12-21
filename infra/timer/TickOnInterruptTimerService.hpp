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

        void NextTriggerChanged() override;
        TimePoint Now() const override;
        Duration Resolution() const override;

        void SetResolution(Duration resolution);

        void TimeProgressed(Duration amount);
        void SystemTickInterrupt();

    private:
        void CalculateNextTrigger();
        void ProcessTicks();

    private:
        TimePoint systemTime = TimePoint();
        Duration resolution;

        std::atomic<uint32_t> ticksNextNotification{ 0 };
        std::atomic<uint32_t> ticksProgressed{ 0 };
        std::atomic_bool notificationScheduled{ false };
    };
}

#endif
