#include "services/ble/BondStorageInteractor.hpp"

namespace services
{

    BondStorageInteractor::BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser, uint32_t maxBondsForThisRole)
        : role(role)
        , bondStorageSynchroniser(bondStorageSynchroniser)
        , maxBondsForThisRole(maxBondsForThisRole)
    {
        bondStorageSynchroniser.AllocateInteractableBondStorage(maxBondsForThisRole);
    }

    BondStorageInteractor::BondStorageInteractor(Role role, BondStorageSynchronizer& bondStorageSynchroniser)
        : BondStorageInteractor(role, bondStorageSynchroniser, bondStorageSynchroniser.GetMaxNumberOfBonds())
    {
    }

    void BondStorageInteractor::AddBond(const services::Bond& bond)
    {
        really_assert_with_msg(GetNumberOfBonds() < maxBondsForThisRole, "AddBond: %d >= %d", GetNumberOfBonds(), maxBondsForThisRole); // TODO: Temporary verbose assert
        really_assert(!bondStorageSynchroniser.GetBond(role, bond.address).has_value());
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

    std::optional<services::Bond> BondStorageInteractor::GetBond(services::GapAddress address)
    {
        return bondStorageSynchroniser.GetBond(role, address);
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
        return maxBondsForThisRole;
    }

    bool BondStorageInteractor::Full() const
    {
        return GetNumberOfBonds() >= maxBondsForThisRole;
    }

    void BondStorageInteractor::AssertBondStoragesAreInSync()
    {
        bondStorageSynchroniser.AssertBondStoragesAreInSyncForRole(role);
    }

    void BondStorageInteractor::RemoveLeastRecentlyUsedBond()
    {
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
