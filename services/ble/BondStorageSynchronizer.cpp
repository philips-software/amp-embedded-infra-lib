#include "services/ble/BondStorageSynchronizer.hpp"
#include "services/ble/Gap.hpp"

namespace services
{
    BondStorageSynchronizerImpl::BondStorageSynchronizerImpl(BondStorageAbsolute& absoluteBondStorage, BondStorage& bondStorage)
        : absoluteBondStorage(absoluteBondStorage)
        , bondStorage(bondStorage)
        , maxNumberOfBonds(std::min(absoluteBondStorage.GetMaxNumberOfBonds(), bondStorage.GetMaxNumberOfBonds()))
    {
        bondStorage.BondStorageSynchronizerCreated(*this);
        absoluteBondStorage.BondStorageSynchronizerCreated(*this);

        really_assert(bondStorage.GetMaxNumberOfBonds() >= absoluteBondStorage.GetMaxNumberOfBonds());

        SyncBondStorages();
    }

    void BondStorageSynchronizerImpl::AddBond(Role role, const services::Bond& bond)
    {
        really_assert(!bondStorage.GetBond(services::Role::central, bond.address).has_value());
        really_assert(!bondStorage.GetBond(services::Role::peripheral, bond.address).has_value());
        bondStorage.AddBond(role, bond);
    }

    void BondStorageSynchronizerImpl::UpdateBondName(Role role, const services::GapAddress& address, infra::BoundedConstString name)
    {
        bondStorage.UpdateBondName(role, address, name);
    }

    void BondStorageSynchronizerImpl::MarkAsRecentlyUsed(Role role, const services::GapAddress& address)
    {
        bondStorage.MarkAsRecentlyUsed(role, address);
    }

    std::optional<services::Bond> BondStorageSynchronizerImpl::GetBond(Role role, const services::GapAddress& address) const
    {
        return bondStorage.GetBond(role, address);
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
        return maxNumberOfBonds;
    }

    void BondStorageSynchronizerImpl::IterateBondedDevices(Role role, const infra::Function<void(const services::Bond&)>& onBond)
    {
        bondStorage.IterateBondedDevices(role, onBond);
    }

    void BondStorageSynchronizerImpl::AllocateInteractableBondStorage(uint32_t size)
    {
        // TODO: This doesn't prevent two interactors of same role from being created. Do we care?
        really_assert(size <= maxNumberOfBonds - interactableBondStorage);
        interactableBondStorage += size;
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
                    absoluteBondStorage.RemoveBond(address);
            });
    }
}
