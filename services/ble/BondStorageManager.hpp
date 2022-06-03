#ifndef SERVICES_BOND_STORAGE_MANAGER_HPP
#define SERVICES_BOND_STORAGE_MANAGER_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/Function.hpp"

namespace application
{
    class BondStorageManager
    {
    public:
        virtual void UpdateBondedDevice(hal::MacAddress address) = 0;
        virtual void RemoveBond(hal::MacAddress address) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
    };

    class BondStorage
    {
    public:
        virtual void BondStorageManagerCreated(BondStorageManager& manager) = 0;
        virtual void UpdateBondedDevice(hal::MacAddress address) = 0;
        virtual void RemoveBond(hal::MacAddress address) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual void RemoveBondIf(infra::Function<bool(hal::MacAddress)> onAddress) = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
        virtual bool IsBondStored(hal::MacAddress address) = 0;
        virtual void GetBondedDevices(infra::Function<void(hal::MacAddress)> onAddress) = 0;
    };

    class BondStorageManagerImpl
        : public BondStorageManager
    {
    public:
        BondStorageManagerImpl(BondStorage& referencebondStorage, BondStorage& otherbondStorage);

        // Implementation of BondStorageManager
        virtual void UpdateBondedDevice(hal::MacAddress address) override;
        virtual void RemoveBond(hal::MacAddress address) override;
        virtual void RemoveAllBonds() override;
        virtual uint32_t GetMaxNumberOfBonds() const override;

    private:
        void SyncBondStorages();

    private:
        BondStorage& referencebondStorage;
        BondStorage& otherbondStorage;
        uint32_t maxNumberOfBonds;
    };
}

#endif
