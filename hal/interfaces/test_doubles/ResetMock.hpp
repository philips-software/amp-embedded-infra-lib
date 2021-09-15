#ifndef HAL_RESET_MOCK_HPP
#define HAL_RESET_MOCK_HPP

#include "hal/interfaces/Reset.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class ResetMock
        : public Reset
    {
    public:
        MOCK_METHOD1(ResetModule, void(const char* resetReason));
    };
}

#endif
