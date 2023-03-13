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
        MOCK_METHOD(void, StateUpdated, (GapState state));
    };
}

#endif
