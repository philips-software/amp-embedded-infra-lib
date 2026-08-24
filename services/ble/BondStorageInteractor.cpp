#include "services/ble/BondStorageInteractor.hpp"

namespace services
{

    BondStorageInteractor::BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser)
        : role(role)
        , bondStorageSynchroniser(bondStorageSynchroniser)
    {}

    void BondStorageInteractor::UpdateBond(const services::Bond& bond)
    {
        // TODO: Here we should evict if full

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

    void BondStorageInteractor::IterateBondedDevices(const infra::Function<void(const services::Bond&)>& onBond)
    {
        bondStorageSynchroniser.IterateBondedDevices(role, onBond);
    }

    uint32_t BondStorageInteractor::GetNumberOfBonds() const
    {
        return bondStorageSynchroniser.GetNumberOfBondsForRole(role);
    }

    uint32_t BondStorageInteractor::GetMaxNumberOfBonds() const
    {
        return bondStorageSynchroniser.GetMaxNumberOfBonds();
    }
}
