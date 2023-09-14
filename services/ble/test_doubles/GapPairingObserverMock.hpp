#ifndef SERVICES_GAP_PAIRING_OBSERVER_MOCK_HPP
#define SERVICES_GAP_PAIRING_OBSERVER_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapPairingObserverMock
        : public GapPairingObserver
    {
    public:
        using GapPairingObserver::GapPairingObserver;

        MOCK_METHOD(void, DisplayPasskey, (int32_t passkey, bool numericComparison), (override));
        MOCK_METHOD(void, PairingSuccessfullyCompleted, (), (override));
        MOCK_METHOD(void, PairingFailed, (PairingErrorType error), (override));
    };
}

#endif
