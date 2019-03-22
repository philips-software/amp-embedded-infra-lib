#include "infra/timer/TimerService.hpp"
#include "infra/timer/TimerServiceManager.hpp"

namespace infra
{
    TimerService::TimerService(uint32_t id)
        : id(id)
    {
        TimerServiceManager::Instance().RegisterTimerService(*this);
    }

    TimerService::~TimerService()
    {
        TimerServiceManager::Instance().UnregisterTimerService(*this);
    }

    uint32_t TimerService::Id() const
    {
        return id;
    }

    void TimerService::RegisterTimer(Timer& timer)
    {
        scheduledTimers.push_front(timer);

        if (timer.NextTrigger() < nextTrigger)
        {
            nextTrigger = timer.NextTrigger();
            NextTriggerChanged();
        }
    }

    void TimerService::UnregisterTimer(Timer& timer, TimePoint oldTriggerTime)
    {
        if (&*timerIterator == &timer)
            ++timerIterator;

        scheduledTimers.erase_slow(timer);

        if (oldTriggerTime == nextTrigger)
            ComputeNextTrigger();
    }

    void TimerService::UpdateTriggerTime(Timer& timer, TimePoint oldTriggerTime)
    {
        if (nextTrigger == oldTriggerTime)
            ComputeNextTrigger();
    }

    void TimerService::Progressed(TimePoint time)
    {
        holdUpdate = true;

        for (timerIterator = scheduledTimers.begin(); timerIterator != scheduledTimers.end(); )
        {
            infra::IntrusiveForwardList<Timer>::iterator currentIterator = timerIterator;
            ++timerIterator;

            if (currentIterator->NextTrigger() <= time)
            {
                infra::Function<void()> action = currentIterator->Action();
                currentIterator->ComputeNextTriggerTime();
                action();
            }
        }

        holdUpdate = false;
        if (updateNeeded)
            ComputeNextTrigger();
    }

    TimePoint TimerService::NextTrigger() const
    {
        return nextTrigger;
    }

    void TimerService::NextTriggerChanged()
    {}

    void TimerService::ComputeNextTrigger()
    {
        if (!holdUpdate)
        {
            updateNeeded = false;
            TimePoint oldTrigger = nextTrigger;

            nextTrigger = TimePoint::max();
            for (auto& timer : scheduledTimers)
                nextTrigger = std::min(nextTrigger, timer.NextTrigger());

            if (nextTrigger != oldTrigger)
                NextTriggerChanged();
        }
        else
            updateNeeded = true;
    }
}
