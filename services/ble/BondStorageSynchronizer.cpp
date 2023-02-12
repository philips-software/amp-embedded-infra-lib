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

        if (otherBondStorage.GetMaxNumberOfBonds() < maxNumberOfBonds)
            std::abort();

        SyncBondStorages();
    }

    void BondStorageSynchronizerImpl::UpdateBondedDevice(hal::MacAddress address)
    {
        referenceBondStorage.UpdateBondedDevice(address);
        otherBondStorage.UpdateBondedDevice(address);
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
            { return !referenceBondStorage.IsBondStored(address); });

        referenceBondStorage.IterateBondedDevices([this](hal::MacAddress address)
            {
                if (!otherBondStorage.IsBondStored(address))
                    otherBondStorage.UpdateBondedDevice(address); });
    }
}
