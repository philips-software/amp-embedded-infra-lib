#include "hal/generic/TimeKeeperGeneric.hpp"

namespace hal
{
    TimeKeeperGeneric::TimeKeeperGeneric(infra::Duration duration)
        : duration(duration)
    {
        Reset();
    }

    bool TimeKeeperGeneric::Timeout()
    {
        return (infra::Now() - startTime) > duration;
    }

    void TimeKeeperGeneric::Reset()
    {
        startTime = infra::Now();
    }
}