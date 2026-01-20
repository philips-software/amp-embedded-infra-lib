#ifndef HAL_BLE_DTM_MOCK_HPP
#define HAL_BLE_DTM_MOCK_HPP

#include "hal/interfaces/BleDtm.hpp"
#include "gmock/gmock.h"
#include <cstdint>
#include <optional>

namespace hal
{
    class BleDtmMock
        : public BleDtm
    {
    public:
        MOCK_METHOD(bool, StartTone, (uint8_t rfChannel, uint8_t offset), (override));
        MOCK_METHOD(bool, StopTone, (), (override));
        MOCK_METHOD(bool, SetTxPowerLevel, (uint8_t txPower), (override));
        MOCK_METHOD(bool, StartRxTest, (uint8_t frequency, uint8_t phy), (override));
        MOCK_METHOD(bool, StartTxTest, (uint8_t frequency, uint8_t dataLength, uint8_t packetPayload, uint8_t phy), (override));
        MOCK_METHOD(std::optional<uint16_t>, StopTest, (), (override));
    };
}

#endif
