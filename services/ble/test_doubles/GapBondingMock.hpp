#ifndef SERVICES_GAP_BONDING_MOCK_HPP
#define SERVICES_GAP_BONDING_MOCK_HPP

#include "services/ble/Gap.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GapPeripheralBondingMock
        : public GapBonding
    {
    public:
        MOCK_METHOD(void, RemoveAllBonds, ());
        MOCK_METHOD(void, RemoveOldestBond, ());
        MOCK_METHOD(std::size_t, GetMaxNumberOfBonds, (), (const));
        MOCK_METHOD(std::size_t, GetNumberOfBonds, (), (const));
    };
}

#endif
