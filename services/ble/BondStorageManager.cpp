#include "services/ble/BondStorageManager.hpp"

namespace services
{
    BondStorageManagerImpl::BondStorageManagerImpl(BondStorage& referenceBondStorage, BondStorage& otherBondStorage)
        : referenceBondStorage(referenceBondStorage)
        , otherBondStorage(otherBondStorage)
        , maxNumberOfBonds(referenceBondStorage.GetMaxNumberOfBonds())
    {
        otherBondStorage.BondStorageManagerCreated(*this);
        referenceBondStorage.BondStorageManagerCreated(*this);

        if (otherBondStorage.GetMaxNumberOfBonds() < maxNumberOfBonds)
            std::abort();

        SyncBondStorages();
    }

    void BondStorageManagerImpl::UpdateBondedDevice(hal::MacAddress address)
    {
        referenceBondStorage.UpdateBondedDevice(address);
        otherBondStorage.UpdateBondedDevice(address);
    }

    void BondStorageManagerImpl::RemoveBond(hal::MacAddress address)
    {
        referenceBondStorage.RemoveBond(address);
        otherBondStorage.RemoveBond(address);
    }

    void BondStorageManagerImpl::RemoveAllBonds()
    {
        referenceBondStorage.RemoveAllBonds();
        otherBondStorage.RemoveAllBonds();
    }

    uint32_t BondStorageManagerImpl::GetMaxNumberOfBonds() const
    {
        return maxNumberOfBonds;
    }

    void BondStorageManagerImpl::SyncBondStorages()
    {
        otherBondStorage.RemoveBondIf([this](hal::MacAddress address)
            {
                return !referenceBondStorage.IsBondStored(address);
            });

        referenceBondStorage.IterateBondedDevices([this](hal::MacAddress address)
            {
                if (!otherBondStorage.IsBondStored(address))
                    otherBondStorage.UpdateBondedDevice(address);
            });
    }
}
