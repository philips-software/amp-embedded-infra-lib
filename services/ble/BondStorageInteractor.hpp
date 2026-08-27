#ifndef SERVICES_BOND_STORAGE_INTERACTOR_HPP
#define SERVICES_BOND_STORAGE_INTERACTOR_HPP

#include "services/ble/BondStorageSynchronizer.hpp"
#include "services/ble/Gap.hpp"

namespace services
{
    class BondStorageInteractor
    {
    public:
        BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser, uint32_t maxNumberOfBonds = 0);
        void AddBond(const services::Bond& bond);
        void UpdateBondName(services::GapAddress address, infra::BoundedConstString name);
        void MarkAsRecentlyUsed(services::GapAddress address);
        void RemoveBond(services::GapAddress address);
        void RemoveAllBonds();
        void IterateBondedDevices(const infra::Function<void(const services::Bond&)>& onBond);

        // TODO: There's still some abiguity on what these mean and what they represent
        uint32_t GetNumberOfBonds() const;
        uint32_t GetMaxNumberOfBonds() const;

    private:
        void RemoveLeastRecentlyUsedBond();

    private:
        Role role;
        BondStorageSynchronizer& bondStorageSynchroniser;
        uint32_t maxNumberOfBonds;
    };
}

#endif
