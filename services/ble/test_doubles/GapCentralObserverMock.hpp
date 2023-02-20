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
        MOCK_METHOD(void, AuthenticationComplete, (GapCentralAuthenticationStatus status), (override));
        MOCK_METHOD(void, DeviceDiscovered, (const GapCentralDiscoveryParameters& deviceDiscovered), (override));
        MOCK_METHOD(void, StateUpdated, (GapCentralState state), (override));
    };
}

#endif
