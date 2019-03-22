#ifndef SYNCHRONOUS_HAL_TIME_SERVICE_HPP
#define SYNCHRONOUS_HAL_TIME_SERVICE_HPP

#include "infra/timer/Timer.hpp"

namespace hal
{
    class TimeService
    {
    public:
        virtual infra::TimePoint Time() = 0;
    };
}

#endif
