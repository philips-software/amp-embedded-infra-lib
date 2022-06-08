#include "services/ble/BondStorageManager.hpp"
#include <algorithm>

namespace services
{
    BondStorageManagerImpl::BondStorageManagerImpl(BondStorage& referencebondStorage, BondStorage& otherbondStorage)
        : referencebondStorage(referencebondStorage)
        , otherbondStorage(otherbondStorage)
        , maxNumberOfBonds(referencebondStorage.GetMaxNumberOfBonds())
    {
        otherbondStorage.BondStorageManagerCreated(*this);
        referencebondStorage.BondStorageManagerCreated(*this);

        if (otherbondStorage.GetMaxNumberOfBonds() < maxNumberOfBonds)
            std::abort();

        SyncBondStorages();
    }

    void BondStorageManagerImpl::UpdateBondedDevice(hal::MacAddress address)
    {
        referencebondStorage.UpdateBondedDevice(address);
        otherbondStorage.UpdateBondedDevice(address);
    }

    void BondStorageManagerImpl::RemoveBond(hal::MacAddress address)
    {
        referencebondStorage.RemoveBond(address);
        otherbondStorage.RemoveBond(address);
    }

    void BondStorageManagerImpl::RemoveAllBonds()
    {
        referencebondStorage.RemoveAllBonds();
        otherbondStorage.RemoveAllBonds();
    }

    uint32_t BondStorageManagerImpl::GetMaxNumberOfBonds() const
    {
        return maxNumberOfBonds;
    }

    void BondStorageManagerImpl::SyncBondStorages()
    {
        otherbondStorage.RemoveBondIf([this](hal::MacAddress address)
        {
            return !referencebondStorage.IsBondStored(address);
        });

        referencebondStorage.IterateBondedDevices([this](hal::MacAddress address)
        {
            if (!otherbondStorage.IsBondStored(address))
                otherbondStorage.UpdateBondedDevice(address);
        });
    }
}