#ifndef SERVICES_BOND_STORAGE_INTERACTOR_HPP
#define SERVICES_BOND_STORAGE_INTERACTOR_HPP

#include "services/ble/BondStorageSynchronizer.hpp"
#include "services/ble/Gap.hpp"

namespace services
{
    class BondStorageInteractor
    {
    public:
        BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser);
        void UpdateBond(const services::Bond& bond);
        void RemoveBond(services::GapAddress address);
        void RemoveAllBonds();
        void IterateBondedDevices(const infra::Function<void(const services::Bond&)>& onBond);
        uint32_t GetNumberOfBonds() const;
        uint32_t GetMaxNumberOfBonds() const;

    private:
        Role role;
        BondStorageSynchronizer& bondStorageSynchroniser;
    };
}

#endif
