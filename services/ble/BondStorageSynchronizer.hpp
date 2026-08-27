#ifndef SERVICES_BOND_STORAGE_SYNCHRONIZER_HPP
#define SERVICES_BOND_STORAGE_SYNCHRONIZER_HPP

#include "infra/util/Function.hpp"
#include "services/ble/Gap.hpp"
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
        virtual void AddBond(Role role, const services::Bond& bond) = 0;
        virtual void UpdateBondName(Role role, const services::GapAddress& address, infra::BoundedConstString name) = 0;
        virtual void MarkAsRecentlyUsed(Role role, const services::GapAddress& address) = 0;
        virtual std::optional<services::Bond> GetBond(Role role, const services::GapAddress& address) const = 0;
        virtual void RemoveBond(Role role, const services::GapAddress& address) = 0;
        virtual void RemoveAllBondsForRole(Role role) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual uint32_t GetNumberOfBondsForRole(Role role) const = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
        virtual void AllocateInteractableBondStorage(uint32_t size) = 0;

        // Iteration is in least recently used order, i.e. the first bond returned is the least recently used one.
        virtual void IterateBondedDevices(Role role, const infra::Function<void(const services::Bond&)>& onBond) = 0;
    };

    class BondStorage
    {
    protected:
        BondStorage() = default;
        BondStorage(const BondStorage& other) = delete;
        BondStorage& operator=(const BondStorage& other) = delete;

    public:
        virtual void BondStorageSynchronizerCreated(BondStorageSynchronizer& manager) = 0;
        virtual void AddBond(Role role, const services::Bond& bond) = 0;
        virtual void UpdateBondName(Role role, const services::GapAddress& address, infra::BoundedConstString name) = 0;
        virtual void MarkAsRecentlyUsed(Role role, const services::GapAddress& address) = 0;
        virtual void RemoveBond(Role role, const services::GapAddress& address) = 0;
        virtual void RemoveAllBondsForRole(Role role) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual void RemoveBondIf(const infra::Function<bool(const services::Bond&)>& onBond) = 0;
        virtual uint32_t GetNumberOfBondsForRole(const Role role) const = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
        virtual std::optional<services::Bond> GetBond(Role role, const services::GapAddress& address) const = 0;

        // Iteration is in least recently used order, i.e. the first bond returned is the least recently used one.
        virtual void IterateBondedDevices(Role role, const infra::Function<void(const services::Bond&)>& onBond) = 0;
    };

    class BondStorageAbsolute
    {
    protected:
        BondStorageAbsolute() = default;
        BondStorageAbsolute(const BondStorageAbsolute& other) = delete;
        BondStorageAbsolute& operator=(const BondStorageAbsolute& other) = delete;

    public:
        virtual void BondStorageSynchronizerCreated(BondStorageSynchronizer& manager) = 0;
        virtual void RemoveBond(const services::GapAddress& address) = 0;
        virtual void RemoveAllBonds() = 0;
        virtual void RemoveBondIf(const infra::Function<bool(const services::GapAddress&)>& onAddress) = 0;
        virtual uint32_t GetMaxNumberOfBonds() const = 0;
        virtual bool IsBondStored(const services::GapAddress& address) const = 0;
        virtual void IterateBondedDevices(const infra::Function<void(const services::GapAddress&)>& onBond) = 0;
    };

    class BondStorageSynchronizerImpl
        : public BondStorageSynchronizer
    {
    public:
        BondStorageSynchronizerImpl(BondStorageAbsolute& absoluteBondStorage, BondStorage& bondStorage);

        // Implementation of BondStorageSynchronizer
        void AddBond(Role role, const services::Bond& bond) override;
        void UpdateBondName(Role role, const services::GapAddress& address, infra::BoundedConstString name) override;
        void MarkAsRecentlyUsed(Role role, const services::GapAddress& address) override;
        std::optional<services::Bond> GetBond(Role role, const services::GapAddress& address) const override;
        void RemoveBond(Role role, const services::GapAddress& address) override;
        void RemoveAllBondsForRole(Role role) override;
        void RemoveAllBonds() override;
        uint32_t GetNumberOfBondsForRole(Role role) const override;
        uint32_t GetMaxNumberOfBonds() const override;
        void IterateBondedDevices(Role role, const infra::Function<void(const services::Bond&)>& onBond) override;
        void AllocateInteractableBondStorage(uint32_t size) override;

    private:
        void SyncBondStorages();

    private:
        BondStorageAbsolute& absoluteBondStorage;
        BondStorage& bondStorage;

        uint32_t maxNumberOfBonds;
        uint32_t interactableBondStorage = 0;
    };
}

#endif
