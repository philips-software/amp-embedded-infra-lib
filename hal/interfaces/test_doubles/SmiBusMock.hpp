#ifndef HAL_SMI_BUS_MOCK_HPP
#define HAL_SMI_BUS_MOCK_HPP

#include "hal/interfaces/SmiBus.hpp"
#include <gmock/gmock.h>

namespace hal
{
    class SmiBusMock
        : public SmiBus
    {
    public:
        MOCK_METHOD(uint16_t, Read, (uint8_t phyAddress, uint16_t reg), (const override));
        MOCK_METHOD(void, Write, (uint8_t phyAddress, uint16_t reg, uint16_t value), (override));
    };
}

#endif
