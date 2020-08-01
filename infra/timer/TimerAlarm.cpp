#include "infra/timer/PartitionedTime.hpp"
#include "infra/timer/TimerAlarm.hpp"

namespace infra
{
    TimerAlarm::TimerAlarm(uint32_t timerServiceId)
        : Timer(timerServiceId)
    {}

    TimerAlarm::TimerAlarm(Duration durationFromMidnight, const infra::Function<void()>& action, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(durationFromMidnight, action);
    }

    void TimerAlarm::Start(Duration durationFromMidnight, const infra::Function<void()>& action)
    {
        this->durationFromMidnight = durationFromMidnight;
        ComputeNextTriggerTime(action);
    }

    void TimerAlarm::ComputeNextTriggerTime()
    {
        ComputeNextTriggerTime(Action());
    }

    void TimerAlarm::Jumped(TimePoint from, TimePoint to)
    {
        auto triggerToday = StartOfDay(to) + durationFromMidnight;

        if (triggerToday >= to)
            SetNextTriggerTime(triggerToday, Action());
        else
            SetNextTriggerTime(triggerToday + std::chrono::hours(24), Action());
    }

    void TimerAlarm::ComputeNextTriggerTime(const infra::Function<void()>& action)
    {
        auto now = Now();
        auto triggerToday = StartOfDay(now) + durationFromMidnight;

        if (triggerToday > now)
            SetNextTriggerTime(triggerToday, action);
        else
            SetNextTriggerTime(triggerToday + std::chrono::hours(24), action);
    }

    TimePoint TimerAlarm::StartOfDay(TimePoint time) const
    {
        PartitionedTime startOfDay(time);
        startOfDay.hours = 0;
        startOfDay.minutes = 0;
        startOfDay.seconds = 0;

        return startOfDay.ToTimePoint();
    }
}
