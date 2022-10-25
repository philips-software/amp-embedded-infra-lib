#include "services/ble/PersistingBondStorage.hpp"

namespace services
{
    PersistingBondStorage::PersistingBondStorage(services::ConfigurationStoreAccess<infra::BoundedVector<uint8_t>> flashStorage, infra::ByteRange ramStorage)
        : flashStorage(flashStorage)
        , ramStorage(ramStorage)
    {
        infra::Copy(infra::MakeRange(*flashStorage), ramStorage);
    }

    void PersistingBondStorage::Update()
    {
        really_assert(ramStorage.size() <= flashStorage->max_size());
        flashStorage->assign(ramStorage.begin(), ramStorage.begin() + ramStorage.size());
        flashStorage.Write();
    }
}
