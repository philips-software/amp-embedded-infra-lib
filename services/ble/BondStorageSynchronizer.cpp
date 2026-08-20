#include "services/ble/BondStorageSynchronizer.hpp"

namespace services
{
    BondStorageSynchronizerImpl::BondStorageSynchronizerImpl(BondStorageAbsolute& absoluteBondStorage, BondStorage& bondStorage)
        : absoluteBondStorage(absoluteBondStorage)
        , bondStorage(bondStorage)
        , maxNumberOfBonds(absoluteBondStorage.GetMaxNumberOfBonds())
    {
        bondStorage.BondStorageSynchronizerCreated(*this);
        absoluteBondStorage.BondStorageSynchronizerCreated(*this);

        really_assert(bondStorage.GetMaxNumberOfBonds() >= maxNumberOfBonds);

        SyncBondStorages();
    }

    void BondStorageSynchronizerImpl::UpdateBond(Role role, const services::Bond& bond)
    {
        bondStorage.UpdateBond(role, bond);
    }

    void BondStorageSynchronizerImpl::RemoveBond(Role role, const services::GapAddress& address)
    {
        absoluteBondStorage.RemoveBond(address);
        bondStorage.RemoveBond(role, address);
    }

    void BondStorageSynchronizerImpl::RemoveAllBondsForRole(Role role)
    {
        bondStorage.IterateBondedDevices([this, role](Role thisBondsRole, const services::Bond& bond)
            {
                if (thisBondsRole == role)
                    absoluteBondStorage.RemoveBond(bond.address);
            });
        bondStorage.RemoveAllBondsForRole(role);
    }

    void BondStorageSynchronizerImpl::RemoveAllBonds()
    {
        absoluteBondStorage.RemoveAllBonds();
        bondStorage.RemoveAllBonds();
    }

    uint32_t BondStorageSynchronizerImpl::GetMaxNumberOfBonds() const
    {
        return maxNumberOfBonds;
    }

    void BondStorageSynchronizerImpl::IterateBondedDevices(const infra::Function<void(const services::Bond&)>& onBond)
    {
        bondStorage.IterateBondedDevices(onBond);
    }

    void BondStorageSynchronizerImpl::SyncBondStorages()
    {
        bondStorage.RemoveBondIf([this](Role, const services::GapAddress& address)
            {
                return !absoluteBondStorage.IsBondStored(address);
            });

        absoluteBondStorage.IterateBondedDevices([this](const services::Bond& bond)
            {
                const auto bondIsStored =
                    bondStorage.GetBond(Role::central, bond.address).has_value() ||
                    bondStorage.GetBond(Role::peripheral, bond.address).has_value();

                if (!bondIsStored)
                {
                    if (hardcodedRole.has_value())
                        bondStorage.UpdateBond(hardcodedRole.value(), bond);
                    else
                        absoluteBondStorage.RemoveBond(bond.address);
                }
            });
    }
}
