#ifndef SERVICES_GAP_PERIPHERALOBSERVER_MOCK_HPP
#define SERVICES_GAP_PERIPHERALOBSERVER_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapPeripheralObserverMock
        : public GapPeripheralObserver
    {
        using GapPeripheralObserver::GapPeripheralObserver;

    public:
        MOCK_METHOD(void, StateChanged, (GapState state));
        MOCK_METHOD(void, ConnectionIntervalUpdated, (uint16_t connMultiplierMin, uint16_t connMultiplierMax));
    };
}

#endif
