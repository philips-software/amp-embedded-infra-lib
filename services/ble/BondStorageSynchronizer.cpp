#include "services/ble/BondStorageSynchronizer.hpp"

namespace services
{
    BondStorageSynchronizerImpl::BondStorageSynchronizerImpl(BondStorage& referenceBondStorage, BondStorage& otherBondStorage)
        : referenceBondStorage(referenceBondStorage)
        , otherBondStorage(otherBondStorage)
        , maxNumberOfBonds(referenceBondStorage.GetMaxNumberOfBonds())
    {
        otherBondStorage.BondStorageSynchronizerCreated(*this);
        referenceBondStorage.BondStorageSynchronizerCreated(*this);

        really_assert(otherBondStorage.GetMaxNumberOfBonds() >= maxNumberOfBonds);

        SyncBondStorages();
    }

    void BondStorageSynchronizerImpl::UpdateBond(const services::Bond& bond)
    {
        referenceBondStorage.UpdateBond(bond);
        otherBondStorage.UpdateBond(bond);
    }

    void BondStorageSynchronizerImpl::RemoveBond(hal::MacAddress address)
    {
        referenceBondStorage.RemoveBond(address);
        otherBondStorage.RemoveBond(address);
    }

    void BondStorageSynchronizerImpl::RemoveAllBonds()
    {
        referenceBondStorage.RemoveAllBonds();
        otherBondStorage.RemoveAllBonds();
    }

    uint32_t BondStorageSynchronizerImpl::GetMaxNumberOfBonds() const
    {
        return maxNumberOfBonds;
    }

    void BondStorageSynchronizerImpl::SyncBondStorages()
    {
        otherBondStorage.RemoveBondIf([this](hal::MacAddress address)
            {
                return !referenceBondStorage.IsBondStored(address);
            });

        referenceBondStorage.IterateBondedDevices([this](const services::Bond& bond)
            {
                if (!otherBondStorage.IsBondStored(bond.address.address))
                    otherBondStorage.UpdateBond(bond);
            });
    }
}
