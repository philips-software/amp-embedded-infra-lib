#include "services/ble/BondStorageSynchronizer.hpp"

namespace services
{
    BondStorageSynchronizerImpl::BondStorageSynchronizerImpl(BondStorageAbsolute& absoluteBondStorage, BondStorage& bondStorage)
        : absoluteBondStorage(absoluteBondStorage)
        , bondStorage(bondStorage)
    {
        bondStorage.BondStorageSynchronizerCreated(*this);
        absoluteBondStorage.BondStorageSynchronizerCreated(*this);

        really_assert(bondStorage.GetMaxNumberOfBonds() >= absoluteBondStorage.GetMaxNumberOfBonds());

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
        bondStorage.IterateBondedDevices(role, [this](const services::Bond& bond)
            {
                absoluteBondStorage.RemoveBond(bond.address);
            });
        bondStorage.RemoveAllBondsForRole(role);
    }

    void BondStorageSynchronizerImpl::RemoveAllBonds()
    {
        absoluteBondStorage.RemoveAllBonds();
        bondStorage.RemoveAllBonds();
    }

    uint32_t BondStorageSynchronizerImpl::GetNumberOfBondsForRole(Role role) const
    {
        return bondStorage.GetNumberOfBondsForRole(role);
    }

    uint32_t BondStorageSynchronizerImpl::GetMaxNumberOfBonds() const
    {
        return absoluteBondStorage.GetMaxNumberOfBonds();
    }

    void BondStorageSynchronizerImpl::IterateBondedDevices(Role role, const infra::Function<void(const services::Bond&)>& onBond)
    {
        bondStorage.IterateBondedDevices(role, onBond);
    }

    void BondStorageSynchronizerImpl::SyncBondStorages()
    {
        bondStorage.RemoveBondIf([this](const services::Bond& bond)
            {
                return !absoluteBondStorage.IsBondStored(bond.address);
            });

        absoluteBondStorage.IterateBondedDevices([this](const services::GapAddress& address)
            {
                const auto bondIsStored =
                    bondStorage.GetBond(Role::central, address).has_value() ||
                    bondStorage.GetBond(Role::peripheral, address).has_value();

                if (!bondIsStored)
                {
                    if (hardcodedRole.has_value())
                        // Could we create the bond here?
                        LOG_AND_ABORT_NOT_IMPLEMENTED();
                    else
                        absoluteBondStorage.RemoveBond(address);
                }
            });
    }
}
