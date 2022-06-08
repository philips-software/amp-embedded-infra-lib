#ifndef SERVICES_BOND_STORAGE_MOCK_HPP
#define SERVICES_BOND_STORAGE_MOCK_HPP

#include "gmock/gmock.h"
#include "services/ble/BondStorageManager.hpp"

namespace services
{
    class BondStorageMock
        : public BondStorage
    {
    public:
        MOCK_METHOD1(BondStorageManagerCreated, void(BondStorageManager& manager));
        MOCK_METHOD1(UpdateBondedDevice, void(hal::MacAddress address));
        MOCK_METHOD1(RemoveBond, void(hal::MacAddress address));
        MOCK_METHOD0(RemoveAllBonds, void());
        MOCK_METHOD1(RemoveBondIf, void(const infra::Function<bool(hal::MacAddress)>& onAddress));
        MOCK_CONST_METHOD0(GetMaxNumberOfBonds, uint32_t());
        MOCK_METHOD1(IsBondStored, bool(hal::MacAddress address));
        MOCK_METHOD1(IterateBondedDevices, void(const infra::Function<void(hal::MacAddress)>& onAddress));
    };
}

#endif
