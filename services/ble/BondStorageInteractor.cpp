#include "services/ble/BondStorageInteractor.hpp"

namespace services
{

    BondStorageInteractor::BondStorageInteractor(BondStorageSynchronizer& bondStorageSynchroniser, Role role)
        : bondStorageSynchroniser(bondStorageSynchroniser)
        , role(role)
    {
    }

    void BondStorageInteractor::UpdateBond(const services::Bond& bond)
    {
        bondStorageSynchroniser.IterateBondedDevices([](const auto& bond) {
            
        });
    }
}
