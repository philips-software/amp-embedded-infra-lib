#ifndef HAL_TIMEKEEPER_GENERIC_HPP
#define HAL_TIMEKEEPER_GENERIC_HPP

#include "hal/synchronous_interfaces/TimeKeeper.hpp"
#include "infra/timer/Timer.hpp"

namespace hal
{
    class TimeKeeperGeneric
        : public hal::TimeKeeper
    {
    public:
        explicit TimeKeeperGeneric(infra::Duration duration);

        bool Timeout() override;
        void Reset() override;

    private:
        const infra::Duration duration;
        infra::TimePoint startTime{ infra::Now() };
    };
}

#endif
