#ifndef SERVICES_BOND_STORAGE_MOCK_SYNCHRONIZER_HPP
#define SERVICES_BOND_STORAGE_MOCK_SYNCHRONIZER_HPP

#include "services/ble/BondStorageSynchronizer.hpp"
#include "gmock/gmock.h"

namespace services
{
    class BondStorageMock
        : public BondStorage
    {
    public:
        MOCK_METHOD(void, BondStorageSynchronizerCreated, (BondStorageSynchronizer & manager), (override));
        MOCK_METHOD(void, AddBond, (const services::Bond& bond), (override));
        MOCK_METHOD(void, MarkAsRecentlyUsed, (hal::MacAddress address), (override));
        MOCK_METHOD(void, RemoveBond, (hal::MacAddress address), (override));
        MOCK_METHOD(void, RemoveAllBonds, (), (override));
        MOCK_METHOD(void, RemoveBondIf, (const infra::Function<bool(hal::MacAddress)>& onAddress), (override));
        MOCK_METHOD(uint32_t, GetMaxNumberOfBonds, (), (const, override));
        MOCK_METHOD(bool, IsBondStored, (hal::MacAddress address), (const, override));
        MOCK_METHOD(void, IterateBondedDevices, (const infra::Function<void(const services::Bond&)>& onAddress), (override));
    };

    class BondStorageSynchronizerMock
        : public BondStorageSynchronizer
    {
    public:
        MOCK_METHOD(void, AddBond, (const services::Bond& bond), (override));
        MOCK_METHOD(void, MarkAsRecentlyUsed, (hal::MacAddress address), (override));
        MOCK_METHOD(void, RemoveBond, (hal::MacAddress address), (override));
        MOCK_METHOD(void, RemoveAllBonds, (), (override));
        MOCK_METHOD(uint32_t, GetMaxNumberOfBonds, (), (const, override));
    };
}

#endif
