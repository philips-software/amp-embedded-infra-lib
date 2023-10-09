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
        MOCK_METHOD(void, DeviceDiscovered, (const GapAdvertisingReport& deviceDiscovered), (override));
        MOCK_METHOD(void, StateChanged, (GapState state), (override));
    };
}

#endif
