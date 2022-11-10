#ifndef SERVICES_BOND_STORAGE_MOCK_SYNCHRONIZER_HPP
#define SERVICES_BOND_STORAGE_MOCK_SYNCHRONIZER_HPP

#include "gmock/gmock.h"
#include "services/ble/BondStorageSynchronizer.hpp"

namespace services
{
    class BondStorageMock
        : public BondStorage
    {
    public:
        MOCK_METHOD1(BondStorageSynchronizerCreated, void(BondStorageSynchronizer& manager));
        MOCK_METHOD1(UpdateBondedDevice, void(hal::MacAddress address));
        MOCK_METHOD1(RemoveBond, void(hal::MacAddress address));
        MOCK_METHOD0(RemoveAllBonds, void());
        MOCK_METHOD1(RemoveBondIf, void(const infra::Function<bool(hal::MacAddress)>& onAddress));
        MOCK_CONST_METHOD0(GetMaxNumberOfBonds, uint32_t());
        MOCK_CONST_METHOD1(IsBondStored, bool(hal::MacAddress address));
        MOCK_METHOD1(IterateBondedDevices, void(const infra::Function<void(hal::MacAddress)>& onAddress));
    };

    class BondStorageSynchronizerMock
        : public BondStorageSynchronizer
    {
    public:
        MOCK_METHOD1(UpdateBondedDevice, void(hal::MacAddress address));
        MOCK_METHOD1(RemoveBond, void(hal::MacAddress address));
        MOCK_METHOD0(RemoveAllBonds, void());
        MOCK_CONST_METHOD0(GetMaxNumberOfBonds, uint32_t());
    };
}

#endif
