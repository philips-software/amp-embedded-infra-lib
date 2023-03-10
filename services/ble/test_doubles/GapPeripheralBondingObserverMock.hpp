#ifndef SERVICES_GAP_PERIPHERALOBSERVER_MOCK_HPP
#define SERVICES_GAP_PERIPHERALOBSERVER_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapPeripheralBondingObserverMock
        : public GapPeripheralBondingObserver
    {
        using GapPeripheralBondingObserver::GapPeripheralBondingObserver;

    public:
        MOCK_METHOD(void, NrOfBondsChanged, (size_t nrBonds));
    };
}

#endif
