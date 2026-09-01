#include "services/ble/BondStorageSynchronizer.hpp"
#include "hal/interfaces/MacAddress.hpp"
#include "services/ble/Gap.hpp"
#include "services/tracer/GlobalTracer.hpp"

namespace services
{
    BondStorageSynchronizerImpl::BondStorageSynchronizerImpl(BondStorageAbsolute& absoluteBondStorage, BondStorage& bondStorage)
        : absoluteBondStorage(absoluteBondStorage)
        , bondStorage(bondStorage)
        , maxNumberOfBonds(std::min(absoluteBondStorage.GetMaxNumberOfBonds(), bondStorage.GetMaxNumberOfBonds()))
    {
        bondStorage.BondStorageSynchronizerCreated(*this);
        absoluteBondStorage.BondStorageSynchronizerCreated(*this);

        really_assert(maxNumberOfBonds != 0);
        really_assert(bondStorage.GetMaxNumberOfBonds() >= absoluteBondStorage.GetMaxNumberOfBonds());

        SyncBondStorages();
    }

    void BondStorageSynchronizerImpl::AddBond(Role role, const services::Bond& bond)
    {
        services::GlobalTracer().Trace() << "=============== Adding bond: " << infra::AsLittleEndianMacAddress(bond.address.address);
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

    void BondStorageSynchronizerImpl::AssertBondStoragesAreInSyncForRole(Role)
    {
        bondStorage.IterateBondedDevices(Role::central, [this](const services::Bond& bond)
            {
                const auto bondIsStored = absoluteBondStorage.IsBondStored(bond.address);
                really_assert_with_msg(bondIsStored, "Bond not found in absolute storage");
            });

        absoluteBondStorage.IterateBondedDevices([this](const services::GapAddress& address)
            {
                // TODO: This can desync when multiple roles are being updated concurrently
                const auto bondIsStored =
                    bondStorage.GetBond(Role::peripheral, address).has_value() ||
                    bondStorage.GetBond(Role::central, address).has_value();
                really_assert_with_msg(bondIsStored, "Bond not found in shadow storage");
            });

        // TODO: Do for role specifically.
        services::GlobalTracer().Trace() << "Bonds: shadow " << bondStorage.GetTotalNumberOfBonds() << ", absolute " << absoluteBondStorage.GetNumberOfBonds();
        really_assert_with_msg(bondStorage.GetTotalNumberOfBonds() == absoluteBondStorage.GetNumberOfBonds(),
            "Bond storage desync: shadow %u vs absolute %u", bondStorage.GetTotalNumberOfBonds(), absoluteBondStorage.GetNumberOfBonds());
    }

    void BondStorageSynchronizerImpl::SyncBondStorages()
    {
        bondStorage.RemoveBondIf([this](const services::Bond& bond)
            {
                const auto bondIsStored = absoluteBondStorage.IsBondStored(bond.address);
                if (!bondIsStored)
                    services::GlobalTracer().Trace() << "================================= Removing bond not stored in absolute storage: " << infra::AsLittleEndianMacAddress(bond.address.address); // TODO: Remove
                return !bondIsStored;
            });

        absoluteBondStorage.IterateBondedDevices([this](const services::GapAddress& address)
            {
                const auto bondIsStored =
                    bondStorage.GetBond(Role::central, address).has_value() ||
                    bondStorage.GetBond(Role::peripheral, address).has_value();

                if (!bondIsStored)
                {
                    services::GlobalTracer().Trace() << "================================= Removing bond not stored in shadow storage: " << infra::AsLittleEndianMacAddress(address.address); // TODO: Remove
                    absoluteBondStorage.RemoveBond(address);
                }
            });
    }
}
