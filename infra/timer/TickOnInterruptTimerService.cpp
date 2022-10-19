#include "infra/timer/TickOnInterruptTimerService.hpp"
#include "infra/event/EventDispatcher.hpp"
#include <cassert>

namespace infra
{
    TickOnInterruptTimerService::TickOnInterruptTimerService(uint32_t id, Duration resolution)
        : TimerService(id)
        , resolution(resolution)
    {
        NextTriggerChanged();
    }

    void TickOnInterruptTimerService::NextTriggerChanged()
    {
        infra::TimePoint nextTrigger = NextTrigger();
        if (nextTrigger != infra::TimePoint::max())
        {
            auto durationToNextNotification = nextTrigger > systemTime ? nextTrigger - systemTime : Duration();
            ticksNextNotification = std::min(static_cast<uint32_t>((durationToNextNotification + resolution - Duration(1)) / resolution), std::numeric_limits<uint32_t>::max() / 2);
        }
        else
            ticksNextNotification = std::numeric_limits<uint32_t>::max() / 2; // Once in a while, an update must be scheduled to avoid overflowing ticksNextNotification in the case no timers are scheduled
    }

    TimePoint TickOnInterruptTimerService::Now() const
    {
        return systemTime + ticksProgressed.load() * resolution;
    }

    Duration TickOnInterruptTimerService::Resolution() const
    {
        return resolution;
    }

    void TickOnInterruptTimerService::SetResolution(Duration resolution)
    {
        this->resolution = resolution;
        NextTriggerChanged();
    }

    void TickOnInterruptTimerService::TimeProgressed(Duration amount)
    {
        systemTime += amount;

        Progressed(systemTime);
    }

    void TickOnInterruptTimerService::SystemTickInterrupt()
    {
        ++ticksProgressed;
        if (ticksProgressed >= ticksNextNotification && !notificationScheduled.exchange(true))
            infra::EventDispatcher::Instance().Schedule([this]()
                { ProcessTicks(); });
    }

    void TickOnInterruptTimerService::ProcessTicks()
    {
        TimeProgressed(ticksProgressed.exchange(0) * resolution);
        NextTriggerChanged();

        // If in the meantime ticksProgressed has been increased beyond ticksNextNotification,
        // the event has not been scheduled by the interrupt, so schedule the event here.
        // Use the result of the assign to notificationScheduled, in order to avoid notificationScheduled
        // being set to false, then receiving an interrupt setting it to true, and not handling the newly scheduled
        // event immediately.
        bool reschedule = notificationScheduled = ticksProgressed >= ticksNextNotification;
        if (reschedule)
            infra::EventDispatcher::Instance().Schedule([this]()
                { ProcessTicks(); });
    }
}
