#ifndef HAL_RESET_MOCK_HPP
#define HAL_RESET_MOCK_HPP

#include "gmock/gmock.h"
#include "hal/interfaces/Reset.hpp"

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
