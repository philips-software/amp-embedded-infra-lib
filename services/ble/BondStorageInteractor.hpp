#ifndef SERVICES_BOND_STORAGE_INTERACTOR_HPP
#define SERVICES_BOND_STORAGE_INTERACTOR_HPP

#include "services/ble/BondStorageSynchronizer.hpp"
#include "services/ble/Gap.hpp"

namespace services
{
    class BondStorageInteractor
    {
    public:
        BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser, uint32_t maxBondsForThisRole);
        BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser);

        void AddBond(const services::Bond& bond);
        void UpdateBondName(services::GapAddress address, infra::BoundedConstString name);
        void MarkAsRecentlyUsed(services::GapAddress address);
        std::optional<services::Bond> GetBond(services::GapAddress address);
        void RemoveBond(services::GapAddress address);
        void RemoveAllBonds();
        void IterateBondedDevices(const infra::Function<void(const services::Bond&)>& onBond);
        uint32_t GetNumberOfBonds() const;
        uint32_t GetMaxNumberOfBonds() const;
        bool Full() const;
        void RemoveLeastRecentlyUsedBond();

    private:
        Role role;
        BondStorageSynchronizer& bondStorageSynchroniser;
        uint32_t maxBondsForThisRole;
    };
}

#endif
