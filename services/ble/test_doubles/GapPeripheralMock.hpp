#ifndef SERVICES_GAP_PERIPHERAL_MOCK_HPP
#define SERVICES_GAP_PERIPHERAL_MOCK_HPP

#include "gmock/gmock.h"
#include "services/ble/Gap.hpp"

namespace services
{
    class GapPeripheralMock
        : public GapPeripheral
    {
    public:
        MOCK_METHOD(GapAddress, GetAddress, (), (const));
        MOCK_METHOD(hal::MacAddress, GetIdentityAddress, (), (const));
        MOCK_METHOD(void, SetAdvertisementData, (infra::ConstByteRange data));
        MOCK_METHOD(void, SetScanResponseData, (infra::ConstByteRange data));
        MOCK_METHOD(void, Advertise, (GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier));
        MOCK_METHOD(void, Standby, ());
    };
}

#endif
