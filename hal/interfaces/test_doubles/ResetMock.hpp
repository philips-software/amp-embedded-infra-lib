#ifndef HAL_RESET_MOCK_HPP
#define HAL_RESET_MOCK_HPP

#include "gmock/gmock.h"
#include "hal/interfaces/Reset.hpp"

namespace hal
{
    //TICS -INT#002: A mock or stub may have public data
    //TICS -INT#027: MOCK_METHOD can't add 'virtual' to its signature
    class ResetMock
        : public Reset
    {
    public:
        MOCK_METHOD1(ResetModule, void(const char* resetReason));
    };
}

#endif
