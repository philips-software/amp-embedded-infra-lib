#ifndef SERVICES_BLE_DTM_HPP
#define SERVICES_BLE_DTM_HPP

#include <cstdint>
#include <optional>

namespace services
{
    class Dtm
    {
    public:
        Dtm() = default;
        Dtm(const Dtm& other) = delete;
        Dtm& operator=(const Dtm& other) = delete;
        ~Dtm() = default;

        virtual bool StartTone(uint8_t rfChannel, uint8_t offset) = 0;
        virtual bool StopTone() = 0;
        virtual bool SetTxPowerLevel(uint8_t txPower) = 0;
        virtual bool StartRxTest(uint8_t frequency, uint8_t phy) = 0;
        virtual bool StartTxTest(uint8_t frequency, uint8_t dataLength, uint8_t packetPayload, uint8_t phy) = 0;
        virtual std::optional<uint16_t> StopTest() = 0; // In case of error returns std::nullopt.
    };
}

#endif
