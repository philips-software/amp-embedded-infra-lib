#ifndef HAL_POWER_MODE_MOCK_HPP
#define HAL_POWER_MODE_MOCK_HPP

#include "hal/interfaces/PowerMode.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class PowerModeMock
        : public PowerMode
    {
    public:
        MOCK_METHOD0(EnterStandby, bool());
    };
}

#endif
