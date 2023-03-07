#ifndef SERVICES_BOND_STORAGE_SYNCHRONIZER_HPP
#define SERVICES_BOND_STORAGE_SYNCHRONIZER_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/Function.hpp"

namespace services
{
    class BondStorageSynchronizer
    {
    protected:
        BondStorageSynchronizer() = default;
        BondStorageSynchronizer(const BondStorageSynchronizer& other) = delete;
        BondStorageSynchronizer& operator=(const BondStorageSynchronizer& other) = delete;
        virtual ~BondStorageSynchronizer() = default;

    public:
        virtual void UpdateBondedDevice(hal::MacAddress address) = 0;
        virtual void RemoveBond(hal::MacAddress address) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
    };

    class BondStorage
    {
    protected:
        BondStorage() = default;
        BondStorage(const BondStorage& other) = delete;
        BondStorage& operator=(const BondStorage& other) = delete;
        virtual ~BondStorage() = default;

    public:
        virtual void BondStorageSynchronizerCreated(BondStorageSynchronizer& manager) = 0;
        virtual void UpdateBondedDevice(hal::MacAddress address) = 0;
        virtual void RemoveBond(hal::MacAddress address) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual void RemoveBondIf(const infra::Function<bool(hal::MacAddress)>& onAddress) = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
        virtual bool IsBondStored(hal::MacAddress address) const = 0;
        virtual void IterateBondedDevices(const infra::Function<void(hal::MacAddress)>& onAddress) = 0;
    };

    class BondStorageSynchronizerImpl
        : public BondStorageSynchronizer
    {
    public:
        BondStorageSynchronizerImpl(BondStorage& referenceBondStorage, BondStorage& otherBondStorage);

        // Implementation of BondStorageSynchronizer
        virtual void UpdateBondedDevice(hal::MacAddress address) override;
        virtual void RemoveBond(hal::MacAddress address) override;
        virtual void RemoveAllBonds() override;
        virtual uint32_t GetMaxNumberOfBonds() const override;

    private:
        void SyncBondStorages();

    private:
        BondStorage& referenceBondStorage;
        BondStorage& otherBondStorage;
        uint32_t maxNumberOfBonds;
    };
}

#endif
