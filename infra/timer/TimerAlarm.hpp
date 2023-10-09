#ifndef INFRA_TIMER_ALARM_HPP
#define INFRA_TIMER_ALARM_HPP

#include "infra/timer/Timer.hpp"

// TimerAlarm triggers when a certain time during the day is reached
// This class takes time jumps into account, so that it triggers correctly even when travelling to other locales or when a daylight saving adjustment is applied on the TimerService

namespace infra
{
    class TimerAlarm
        : public Timer
    {
    public:
        explicit TimerAlarm(uint32_t timerServiceId = systemTimerServiceId);
        TimerAlarm(Duration durationFromMidnight, const infra::Function<void()>& action, uint32_t timerServiceId = systemTimerServiceId);

        void Start(Duration durationFromMidnight, const infra::Function<void()>& action);

    public:
        void ComputeNextTriggerTime() override;
        void Jumped(TimePoint from, TimePoint to) override;

    private:
        void ComputeNextTriggerTime(const infra::Function<void()>& action);
        TimePoint StartOfDay(TimePoint time) const;

    private:
        Duration durationFromMidnight{};
    };
}

#endif
