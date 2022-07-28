#include "infra/timer/Timer.hpp"
#include "infra/timer/TimerService.hpp"
#include <cassert>

namespace infra
{
    TimePoint Now(uint32_t timerServiceId)
    {
        return TimerService::GetTimerService(timerServiceId).Now();
    }

    Timer::Timer(uint32_t timerServiceId)
        : timerService(TimerService::GetTimerService(timerServiceId))
    {}

    Timer::~Timer()
    {
        Cancel();
    }

    void Timer::Cancel()
    {
        if (action)
            UnregisterSelf(Convert(nextTriggerTime));

        nextTriggerTime = Convert(TimePoint());
        action = nullptr;
    }

    bool Timer::Armed() const
    {
        return action != nullptr;
    }

    void Timer::Jumped(TimePoint from, TimePoint to)
    {
        // Default behaviour: Just pretend that during the jump no actual time passed
        nextTriggerTime = Convert(Convert(nextTriggerTime) + (to - from));
    }

    TimePoint Timer::NextTrigger() const
    {
        return Convert(nextTriggerTime);
    }

    const infra::Function<void()>& Timer::Action() const
    {
        return action;
    }

    void Timer::RegisterSelf()
    {
        timerService.RegisterTimer(*this);
    }

    void Timer::UnregisterSelf(TimePoint oldTriggerTime)
    {
        timerService.UnregisterTimer(*this, oldTriggerTime);
    }

    void Timer::UpdateTriggerTime(TimePoint oldTriggerTime)
    {
        timerService.UpdateTriggerTime(*this, oldTriggerTime);
    }

    Timer::UnalignedTimePoint Timer::Convert(TimePoint point) const
    {
        UnalignedTimePoint result;
        std::copy(reinterpret_cast<const uint32_t*>(&point), reinterpret_cast<const uint32_t*>(&point + 1), result.begin());
        return result;
    }

    TimePoint Timer::Convert(UnalignedTimePoint point) const
    {
        TimePoint result;
        std::copy(point.begin(), point.end(), reinterpret_cast<uint32_t*>(&result));
        return result;
    }

    void Timer::SetNextTriggerTime(TimePoint time, const infra::Function<void()>& action)
    {
        assert(action);
        TimePoint oldTriggerTime = Convert(nextTriggerTime);

        nextTriggerTime = Convert(time);

        if (!this->action)
            RegisterSelf();
        else
            UpdateTriggerTime(oldTriggerTime);

        this->action = action;
    }

    TimePoint Timer::Now() const
    {
        return timerService.Now();
    }

    Duration Timer::Resolution() const
    {
        return timerService.Resolution();
    }

    TimerSingleShot::TimerSingleShot(uint32_t timerServiceId)
        : Timer(timerServiceId)
    {}

    TimerSingleShot::TimerSingleShot(TimePoint time, const infra::Function<void()>& action, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(time, action);
    }

    TimerSingleShot::TimerSingleShot(Duration duration, const infra::Function<void()>& action, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(duration, action);
    }

    void TimerSingleShot::Start(TimePoint time, const infra::Function<void()>& action)
    {
        if (time < infra::TimePoint::max() - Resolution())
            SetNextTriggerTime(time + Resolution(), action);
        else
            Cancel();
    }

    void TimerSingleShot::Start(Duration duration, const infra::Function<void()>& action)
    {
        SetNextTriggerTime(Now() + duration + Resolution(), action);
    }

    void TimerSingleShot::ComputeNextTriggerTime()
    {
        Cancel();
    }

    TimerRepeating::TimerRepeating(uint32_t timerServiceId)
        : Timer(timerServiceId)
    {}

    TimerRepeating::TimerRepeating(Duration duration, const infra::Function<void()>& aAction, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(duration, aAction);
    }

    TimerRepeating::TimerRepeating(Duration duration, const infra::Function<void()>& aAction, TriggerImmediately, uint32_t timerServiceId)
        : Timer(timerServiceId)
    {
        Start(duration, aAction, triggerImmediately);
    }

    void TimerRepeating::Start(Duration duration, const infra::Function<void()>& action)
    {
        triggerPeriod = duration;
        SetNextTriggerTime(Now() + TriggerPeriod() + Resolution(), action);
    }

    void TimerRepeating::Start(Duration duration, const infra::Function<void()>& action, TriggerImmediately)
    {
        Start(duration, action);
        action();
    }

    Duration TimerRepeating::TriggerPeriod() const
    {
        return triggerPeriod;
    }

    void TimerRepeating::ComputeNextTriggerTime()
    {
        TimePoint now = std::max(Now(), NextTrigger());
        Duration diff = (now - NextTrigger()) % triggerPeriod;

        SetNextTriggerTime(now - diff + triggerPeriod, Action());
    }
}
