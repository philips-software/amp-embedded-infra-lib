#ifndef SERVICES_GAP_PERIPHERAL_BONDING_MOCK_HPP
#define SERVICES_GAP_PERIPHERAL_BONDING_MOCK_HPP

#include "gmock/gmock.h"
#include "services/ble/Gap.hpp"

namespace services
{
    class GapPeripheralBondingMock
        : public GapPeripheralBonding
    {
    public:
        MOCK_METHOD(void, RemoveAllBonds, ());
        MOCK_METHOD(void, RemoveOldestBond, ());
        MOCK_METHOD(size_t, GetMaxNumberOfBonds, (), (const));
        MOCK_METHOD(size_t, GetNumberOfBonds, (), (const));
    };
}

#endif
