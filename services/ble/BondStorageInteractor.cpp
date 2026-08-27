#include "services/ble/BondStorageInteractor.hpp"

namespace services
{

    BondStorageInteractor::BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser, uint32_t maxNumberOfBonds)
        : role(role)
        , bondStorageSynchroniser(bondStorageSynchroniser)
        , maxNumberOfBonds(maxNumberOfBonds)
    {
        // TODO: This does not account for multiple interactors being active.
        really_assert(maxNumberOfBonds <= bondStorageSynchroniser.GetMaxNumberOfBonds());
    }

    void BondStorageInteractor::AddBond(const services::Bond& bond)
    {
        if (bondStorageSynchroniser.GetBond(role, bond.address).has_value())
            // TODO: Should we update the name in this case?
            // TODO: This should never occur? Do we care about that?
            return;

        if (GetNumberOfBonds() >= maxNumberOfBonds)
            RemoveLeastRecentlyUsedBond();

        really_assert(GetNumberOfBonds() < maxNumberOfBonds);
        bondStorageSynchroniser.AddBond(role, bond);
    }

    void BondStorageInteractor::UpdateBondName(services::GapAddress address, infra::BoundedConstString name)
    {
        bondStorageSynchroniser.UpdateBondName(role, address, name);
    }

    void BondStorageInteractor::MarkAsRecentlyUsed(services::GapAddress address)
    {
        bondStorageSynchroniser.MarkAsRecentlyUsed(role, address);
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
        return maxNumberOfBonds;
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
