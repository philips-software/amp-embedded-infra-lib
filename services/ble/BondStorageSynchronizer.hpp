#ifndef SERVICES_BOND_STORAGE_SYNCHRONIZER_HPP
#define SERVICES_BOND_STORAGE_SYNCHRONIZER_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Function.hpp"
#include "services/ble/Gap.hpp"
#include <cstddef>
#include <optional>

namespace services
{
    class BondStorageSynchronizer
    {
    protected:
        BondStorageSynchronizer() = default;
        BondStorageSynchronizer(const BondStorageSynchronizer& other) = delete;
        BondStorageSynchronizer& operator=(const BondStorageSynchronizer& other) = delete;

    public:
        virtual void AddBond(const services::Bond& bond) = 0;
        virtual void MarkAsRecentlyUsed(hal::MacAddress address) = 0;
        virtual void RemoveBond(hal::MacAddress address) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
    };

    class BondNameStorage // TODO: Remove
    {
    protected:
        BondNameStorage() = default;
        ~BondNameStorage() = default;
        BondNameStorage(const BondNameStorage& other) = delete;
        BondNameStorage& operator=(const BondNameStorage& other) = delete;

    public:
        virtual void SetBondName(GapAddress address, infra::BoundedConstString name) = 0;
        virtual std::optional<infra::BoundedConstString> GetBondName(GapAddress address) const = 0;
        virtual void RemoveBondName(GapAddress address) = 0;
        virtual void RemoveAllBondNames() = 0;
        virtual std::size_t Size() const = 0;
        virtual std::size_t Capacity() const = 0;
    };

    class BondStorage
    {
    protected:
        BondStorage() = default;
        BondStorage(const BondStorage& other) = delete;
        BondStorage& operator=(const BondStorage& other) = delete;

    public:
        virtual void BondStorageSynchronizerCreated(BondStorageSynchronizer& manager) = 0;
        virtual void AddBond(const services::Bond& bond) = 0;
        virtual void MarkAsRecentlyUsed(hal::MacAddress address) = 0;
        virtual void RemoveBond(hal::MacAddress address) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual void RemoveBondIf(const infra::Function<bool(hal::MacAddress)>& onAddress) = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
        virtual bool IsBondStored(hal::MacAddress address) const = 0;
        virtual void IterateBondedDevices(const infra::Function<void(const services::Bond&)>& onAddress) = 0;
    };

    class BondStorageSynchronizerImpl
        : public BondStorageSynchronizer
    {
    public:
        BondStorageSynchronizerImpl(BondStorage& referenceBondStorage, BondStorage& otherBondStorage);

        // Implementation of BondStorageSynchronizer
        void AddBond(const services::Bond& bond) override;
        void MarkAsRecentlyUsed(hal::MacAddress address) override;
        void RemoveBond(hal::MacAddress address) override;
        void RemoveAllBonds() override;
        uint32_t GetMaxNumberOfBonds() const override;

    private:
        void SyncBondStorages();

    private:
        BondStorage& referenceBondStorage;
        BondStorage& otherBondStorage;
        uint32_t maxNumberOfBonds;
    };
}

#endif
