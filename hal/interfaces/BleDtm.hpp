#ifndef HAL_INTERFACE_BLE_DTM_HPP
#define HAL_INTERFACE_BLE_DTM_HPP

#include <cstdint>
#include <infra/util/Optional.hpp>

namespace hal
{
    class BleDtm
    {
    public:
        virtual bool StartTone(uint8_t rfChannel, uint8_t offset) = 0;
        virtual bool StopTone() = 0;
        virtual bool SetTxPowerLevel(uint8_t txPower) = 0;
        virtual bool StartRxTest(uint8_t frequency, uint8_t phy) = 0;
        virtual bool StartTxTest(uint8_t frequency, uint8_t dataLength, uint8_t packetPayload, uint8_t phy) = 0;
        virtual infra::Optional<uint16_t> StopTest();
    };
}

#endif
