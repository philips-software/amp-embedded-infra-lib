#include "services/ble/BondStorageManager.hpp"
#include <algorithm>

namespace application
{
    BondStorageManagerImpl::BondStorageManagerImpl(infra::MemoryRange<BondStorage*> bondStorages, uint32_t referenceBondStoragePosition)
        : bondStorages(bondStorages)
        , referenceStorage(bondStorages[referenceBondStoragePosition])
        , maxNumberOfBonds(referenceStorage->GetMaxNumberOfBonds())
    {
        for (auto& bondStorage : bondStorages)
        {
            bondStorage->BondStorageManagerCreated(*this);
            if (bondStorage->GetMaxNumberOfBonds() < maxNumberOfBonds)
                std::abort();
        }

        SyncBondStorages();
    }

    void BondStorageManagerImpl::UpdateBondedDevice(hal::MacAddress address)
    {
        for (auto& bondStorage : bondStorages)
            bondStorage->UpdateBondedDevice(address);
    }

    void BondStorageManagerImpl::RemoveBond(hal::MacAddress address)
    {
        for (auto& bondStorage : bondStorages)
            bondStorage->RemoveBond(address);
    }

    void BondStorageManagerImpl::RemoveAllBonds()
    {
        for (auto& bondStorage : bondStorages)
            bondStorage->RemoveAllBonds();
    }

    uint32_t BondStorageManagerImpl::GetMaxNumberOfBonds() const
    {
        return maxNumberOfBonds;
    }

    void BondStorageManagerImpl::SyncBondStorages()
    {
        for (auto& bondStorage : bondStorages)
        {
            if (bondStorage == referenceStorage)
                continue;

            bondStorage->GetBondedDevices([this, &bondStorage](hal::MacAddress address)
            {
                if (!referenceStorage->IsBondStored(address))
                    bondStorage->RemoveBond(address);
            });
        }

        referenceStorage->GetBondedDevices([this](hal::MacAddress address)
        {
            for (auto& bondStorage : bondStorages)
            {
                if (bondStorage == referenceStorage)
                    continue;

                if (!bondStorage->IsBondStored(address))
                    bondStorage->UpdateBondedDevice(address);
            }
        });
    }
}