#ifndef SERVICES_GAP_PAIRING_MOCK_HPP
#define SERVICES_GAP_PAIRING_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapPeripheralPairingMock
        : public GapPairing
    {
    public:
        MOCK_METHOD(void, AllowPairing, (bool allow));
        MOCK_METHOD(void, SetSecurityMode, (SecurityMode mode, SecurityLevel level));
        MOCK_METHOD(void, SetIoCapabilities, (IoCapabilities caps));
        MOCK_METHOD(void, AuthenticateWithPasskey, (uint32_t passkey));
        MOCK_METHOD(void, NumericComparisonConfirm, (bool accept));
    };
}

#endif
