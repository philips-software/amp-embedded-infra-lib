#include "services/ble/BondStorageSynchronizer.hpp"
#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/LogAndAbort.hpp"
#include "services/ble/Gap.hpp"

namespace
{
    services::Role OppositeRole(services::Role role)
    {
        switch (role)
        {
            case services::Role::central:
                return services::Role::peripheral;
            case services::Role::peripheral:
                return services::Role::central;
            default:
                LOG_AND_ABORT_ENUM(role);
        }
    }
}

namespace services
{
    BondStorageSynchronizerImpl::BondStorageSynchronizerImpl(AuthoritativeBondStorage& authoritativeBondStorage, EnrichedBondStorage& bondStorage)
        : authoritativeBondStorage(authoritativeBondStorage)
        , enrichedBondStorage(bondStorage)
        , maxNumberOfBonds(std::min(authoritativeBondStorage.GetMaxNumberOfBonds(), bondStorage.GetMaxNumberOfBonds()))
    {
        bondStorage.BondStorageSynchronizerCreated(*this);
        authoritativeBondStorage.BondStorageSynchronizerCreated(*this);

        really_assert(maxNumberOfBonds != 0);
        really_assert(bondStorage.GetMaxNumberOfBonds() >= authoritativeBondStorage.GetMaxNumberOfBonds());

        SyncBondStorages();
    }

    void BondStorageSynchronizerImpl::AddBond(Role role, const services::Bond& bond)
    {
        really_assert_with_msg(!enrichedBondStorage.GetBond(services::Role::central, bond.address).has_value(),
            "Bond already exists for role %d", static_cast<int>(services::Role::central));
        really_assert_with_msg(!enrichedBondStorage.GetBond(services::Role::peripheral, bond.address).has_value(),
            "Bond already exists for role %d", static_cast<int>(services::Role::peripheral));
        enrichedBondStorage.AddBond(role, bond);
    }

    void BondStorageSynchronizerImpl::UpdateBondName(Role role, const services::GapAddress& address, infra::BoundedConstString name)
    {
        enrichedBondStorage.UpdateBondName(role, address, name);
    }

    void BondStorageSynchronizerImpl::MarkAsRecentlyUsed(Role role, const services::GapAddress& address)
    {
        enrichedBondStorage.MarkAsRecentlyUsed(role, address);
    }

    std::optional<services::Bond> BondStorageSynchronizerImpl::GetBond(Role role, const services::GapAddress& address) const
    {
        return enrichedBondStorage.GetBond(role, address);
    }

    void BondStorageSynchronizerImpl::RemoveBond(Role role, const services::GapAddress& address)
    {
        authoritativeBondStorage.RemoveBond(address);
        enrichedBondStorage.RemoveBond(role, address);
    }

    void BondStorageSynchronizerImpl::RemoveAllBondsForRole(Role role)
    {
        enrichedBondStorage.IterateBondedDevices(role, [this](const services::Bond& bond)
            {
                authoritativeBondStorage.RemoveBond(bond.address);
            });
        enrichedBondStorage.RemoveAllBondsForRole(role);
    }

    void BondStorageSynchronizerImpl::RemoveAllBonds()
    {
        authoritativeBondStorage.RemoveAllBonds();
        enrichedBondStorage.RemoveAllBonds();
    }

    uint32_t BondStorageSynchronizerImpl::GetNumberOfBondsForRole(Role role) const
    {
        return enrichedBondStorage.GetNumberOfBondsForRole(role);
    }

    uint32_t BondStorageSynchronizerImpl::GetMaxNumberOfBonds() const
    {
        return maxNumberOfBonds;
    }

    void BondStorageSynchronizerImpl::IterateBondedDevices(Role role, const infra::Function<void(const services::Bond&)>& onBond)
    {
        enrichedBondStorage.IterateBondedDevices(role, onBond);
    }

    void BondStorageSynchronizerImpl::AllocateInteractableBondStorage(uint32_t size)
    {
        really_assert(size <= maxNumberOfBonds - interactableBondStorage);
        interactableBondStorage += size;
    }

    void BondStorageSynchronizerImpl::AssertBondStoragesAreInSyncForRole(Role role)
    {
        enrichedBondStorage.IterateBondedDevices(role, [this](const services::Bond& bond)
            {
                const auto bondIsStored = authoritativeBondStorage.IsBondStored(bond.address);
                really_assert_with_msg(bondIsStored, "Bond not found in authoritative storage: %02X:%02X:%02X:%02X:%02X:%02X",
                    static_cast<unsigned int>(bond.address.address[5]), static_cast<unsigned int>(bond.address.address[4]), static_cast<unsigned int>(bond.address.address[3]),
                    static_cast<unsigned int>(bond.address.address[2]), static_cast<unsigned int>(bond.address.address[1]), static_cast<unsigned int>(bond.address.address[0]));
            });

        // Note: cannot verify authoritative storage because it may contain desyncs for the other role,
        // because we can't know the state of the other role's bonds at this point.
    }

    void BondStorageSynchronizerImpl::SyncBondStorages()
    {
        enrichedBondStorage.RemoveBondIf([this](const services::Bond& bond)
            {
                return !authoritativeBondStorage.IsBondStored(bond.address);
            });

        authoritativeBondStorage.RemoveBondIf([this](const services::GapAddress& address)
            {
                return !enrichedBondStorage.GetBond(Role::central, address).has_value() &&
                       !enrichedBondStorage.GetBond(Role::peripheral, address).has_value();
            });
    }
}
