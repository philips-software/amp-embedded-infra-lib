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

        MOCK_METHOD(void, AuthenticationRequired, (bool numericComparison, uint32_t passkey), (override));
        MOCK_METHOD(void, PairingResult, (bool pairedSuccessfully, PairingFailedReason pairingFailedReason), (override));
        MOCK_METHOD(void, OutOfBandDataGenerated, (const GapOutOfBandData& outOfBandData), (override));
    };
}

#endif
