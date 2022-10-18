#include "infra/timer/test_helper/PerfectTimerService.hpp"

namespace infra
{
    PerfectTimerService::PerfectTimerService(uint32_t id)
        : TimerService(id)
    {}

    void PerfectTimerService::NextTriggerChanged()
    {
        nextNotification = static_cast<uint32_t>(std::max<std::chrono::milliseconds::rep>(
            std::chrono::duration_cast<std::chrono::milliseconds>(NextTrigger() - previousTrigger).count(), 0));
    }

    TimePoint PerfectTimerService::Now() const
    {
        return systemTime;
    }

    Duration PerfectTimerService::Resolution() const
    {
        return resolution;
    }

    void PerfectTimerService::SetResolution(Duration resolution)
    {
        this->resolution = resolution;
    }

    void PerfectTimerService::TimeProgressed(Duration amount)
    {
        systemTime += amount;
        Progressed(systemTime);
    }

    void PerfectTimerService::TimeJumped(Duration amount)
    {
        auto oldSystemTime = systemTime;
        systemTime += amount;
        Jumped(oldSystemTime, systemTime);
    }
}
