#ifndef SERVICES_GAP_PERIPHERAL_MOCK_HPP
#define SERVICES_GAP_PERIPHERAL_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapPeripheralMock
        : public GapPeripheral
    {
    public:
        MOCK_METHOD(GapAddress, GetAddress, (), (const));
        MOCK_METHOD(GapAddress, GetIdentityAddress, (), (const));
        MOCK_METHOD(void, SetAdvertisementData, (infra::ConstByteRange data));
        MOCK_METHOD(void, SetScanResponseData, (infra::ConstByteRange data));
        MOCK_METHOD(void, Advertise, (GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier));
        MOCK_METHOD(void, Standby, ());
    };
}

#endif
