#include "services/ble/BondStorageInteractor.hpp"

namespace services
{

    BondStorageInteractor::BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser)
        : role(role)
        , bondStorageSynchroniser(bondStorageSynchroniser)
    {}

    void BondStorageInteractor::UpdateBond(const services::Bond& bond)
    {
        bondStorageSynchroniser.UpdateBond(role, bond);
    }

    void BondStorageInteractor::RemoveBond(services::GapAddress address)
    {
        bondStorageSynchroniser.RemoveBond(role, address);
    }

    void BondStorageInteractor::RemoveAllBonds()
    {
        bondStorageSynchroniser.RemoveAllBondsForRole(role);
    }

    uint32_t BondStorageInteractor::GetMaxNumberOfBonds() const
    {
        return bondStorageSynchroniser.GetMaxNumberOfBonds();
    }
}
