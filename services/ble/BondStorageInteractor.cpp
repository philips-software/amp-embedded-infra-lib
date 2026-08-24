#include "services/ble/BondStorageInteractor.hpp"

namespace services
{

    BondStorageInteractor::BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser, uint32_t maxNumberOfBonds)
        : role(role)
        , bondStorageSynchroniser(bondStorageSynchroniser)
        , maxNumberOfBonds(maxNumberOfBonds)
    {
        // TODO: This does not account for multiple roles being active.
        really_assert(maxNumberOfBonds <= bondStorageSynchroniser.GetMaxNumberOfBonds());
    }

    void BondStorageInteractor::UpdateBond(const services::Bond& bond)
    {
        // TODO: This is not correct. Update also happens for existing bonds.
        if (GetNumberOfBonds() >= maxNumberOfBonds)
            RemoveLeastRecentlyUsedBond();
        really_assert(GetNumberOfBonds() < maxNumberOfBonds);

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

    void BondStorageInteractor::RemoveLeastRecentlyUsedBond()
    {
        // This removes the least recently used bond.
        std::optional<services::Bond> oldestBond;
        bondStorageSynchroniser.IterateBondedDevices(role, [&oldestBond](const services::Bond& bond)
            {
                if (!oldestBond.has_value())
                    oldestBond = bond;
            });

        if (oldestBond.has_value())
            bondStorageSynchroniser.RemoveBond(role, oldestBond->address);
    }
}
