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
        MOCK_METHOD(void, UpdateBond, (Role role, const services::Bond& bond), (override));
        MOCK_METHOD(void, RemoveBond, (Role role, const services::GapAddress& address), (override));
        MOCK_METHOD(void, RemoveAllBondsForRole, (Role role), (override));
        MOCK_METHOD(void, RemoveAllBonds, (), (override));
        MOCK_METHOD(void, RemoveBondIf, (const infra::Function<bool(const services::Bond&)>& onAddress), (override));
        MOCK_METHOD(uint32_t, GetMaxNumberOfBonds, (), (const, override));
        MOCK_METHOD(std::optional<services::Bond>, GetBond, (Role role, const services::GapAddress& address), (const, override));
        MOCK_METHOD(void, IterateBondedDevices, (Role role, const infra::Function<void(const services::Bond&)>& onBond), (override));
    };

    class BondStorageAbsoluteMock
        : public BondStorageAbsolute
    {
    public:
        MOCK_METHOD(void, BondStorageSynchronizerCreated, (BondStorageSynchronizer & manager), (override));
        MOCK_METHOD(void, RemoveBond, (const services::GapAddress& address), (override));
        MOCK_METHOD(void, RemoveAllBonds, (), (override));
        MOCK_METHOD(void, RemoveBondIf, (const infra::Function<bool(const services::GapAddress&)>& onAddress), (override));
        MOCK_METHOD(uint32_t, GetMaxNumberOfBonds, (), (const, override));
        MOCK_METHOD(bool, IsBondStored, (const services::GapAddress& address), (const, override));
        MOCK_METHOD(void, IterateBondedDevices, (const infra::Function<void(const services::GapAddress&)>& onBond), (override));
    };

    class BondStorageSynchronizerMock
        : public BondStorageSynchronizer
    {
    public:
        MOCK_METHOD(void, UpdateBond, (Role role, const services::Bond& bond), (override));
        MOCK_METHOD(void, RemoveBond, (Role role, const services::GapAddress& address), (override));
        MOCK_METHOD(void, RemoveAllBondsForRole, (Role role), (override));
        MOCK_METHOD(void, RemoveAllBonds, (), (override));
        MOCK_METHOD(uint32_t, GetMaxNumberOfBonds, (), (const, override));
        MOCK_METHOD(void, IterateBondedDevices, (Role role, const infra::Function<void(const services::Bond&)>& onBond), (override));
    };
}

#endif
