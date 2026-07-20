#ifndef BLE_TEST_DOUBLES_DTM_MOCK_HPP
#define BLE_TEST_DOUBLES_DTM_MOCK_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "services/ble/Dtm.hpp"
#include "gmock/gmock.h"
#include <cstdint>
#include <optional>

namespace services
{
    class DtmMock
        : public Dtm
    {
    public:
        MOCK_METHOD(bool, StartTone, (uint8_t rfChannel, uint8_t offset), (override));
        MOCK_METHOD(bool, StopTone, (), (override));
        MOCK_METHOD(bool, SetTxPowerLevel, (uint8_t txPower), (override));
        MOCK_METHOD(bool, StartRxTest, (uint8_t frequency, uint8_t phy), (override));
        MOCK_METHOD(bool, StartTxTest, (uint8_t frequency, uint8_t dataLength, uint8_t packetPayload, uint8_t phy), (override));
        MOCK_METHOD(void, StopTest, (const infra::AutoResetFunction<void(std::optional<uint16_t>)>&), (override));
    };
}

#endif
