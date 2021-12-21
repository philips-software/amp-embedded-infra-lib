#ifndef SYNCHRONOUS_HAL_TIMEKEEPER_MOCK_HPP
#define SYNCHRONOUS_HAL_TIMEKEEPER_MOCK_HPP

#include "gmock/gmock.h"
#include "hal/synchronous_interfaces/TimeKeeper.hpp"

namespace hal
{
    class TimeKeeperMock
        : public hal::TimeKeeper
    {
    public:
        MOCK_METHOD0(Timeout, bool());
        MOCK_METHOD0(Reset, void());
    };
}

#endif
