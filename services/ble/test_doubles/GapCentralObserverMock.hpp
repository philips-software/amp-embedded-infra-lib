#ifndef SERVICES_GAP_CENTRAL_OBSERVER_MOCK_HPP
#define SERVICES_GAP_CENTRAL_OBSERVER_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapCentralObserverMock
        : public GapCentralObserver
    {
        using GapCentralObserver::GapCentralObserver;

    public:
        MOCK_METHOD(void, AuthenticationSuccessfullyCompleted, (), (override));
        MOCK_METHOD(void, AuthenticationFailed, (GapAuthenticationErrorType error), (override));
        MOCK_METHOD(void, DeviceDiscovered, (const GapAdvertisingReport& deviceDiscovered), (override));
        MOCK_METHOD(void, StateUpdated, (GapState state), (override));
    };
}

#endif
