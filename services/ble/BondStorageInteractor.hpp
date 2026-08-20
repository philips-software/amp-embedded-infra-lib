#ifndef SERVICES_BOND_STORAGE_INTERACTOR_HPP
#define SERVICES_BOND_STORAGE_INTERACTOR_HPP

#include "services/ble/BondStorageSynchronizer.hpp"
#include "services/ble/Gap.hpp"

namespace services
{
    class BondStorageInteractor
    {
    protected:
        BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser);

    public:
        void UpdateBond(const services::Bond& bond);
        void RemoveBond(services::GapAddress address);
        void RemoveAllBonds();
        uint32_t GetMaxNumberOfBonds() const;

    private:
        Role role;
        BondStorageSynchronizer& bondStorageSynchroniser;
    };
}

#endif
