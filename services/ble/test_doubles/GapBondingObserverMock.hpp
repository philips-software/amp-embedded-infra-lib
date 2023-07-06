#ifndef SERVICES_GAP_OBSERVER_MOCK_HPP
#define SERVICES_GAP_OBSERVER_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapBondingObserverMock
        : public GapBondingObserver
    {
        using GapBondingObserver::GapBondingObserver;

    public:
        MOCK_METHOD(void, NumberOfBondsChanged, (size_t nrBonds));
    };
}

#endif
