#ifndef SERVICES_BOND_STORAGE_INTERACTOR_HPP
#define SERVICES_BOND_STORAGE_INTERACTOR_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "services/ble/BondStorageSynchronizer.hpp"
#include "services/ble/Gap.hpp"

namespace services
{
    class BondStorageInteractor
    {
    protected:
        BondStorageInteractor(BondStorageSynchronizer& bondStorageSynchroniser, Role role);

    public:
        void UpdateBond(const services::Bond& bond);
        void RemoveBond(hal::MacAddress address);
        void RemoveAllBonds();
        uint32_t GetMaxNumberOfBonds() const;

    private:
        BondStorageSynchronizer& bondStorageSynchroniser;
        Role role;
    };
}

#endif
