#include "services/ble/BondBlobPersistence.hpp"

namespace services
{
    BondBlobPersistence::BondBlobPersistence(services::ConfigurationStoreAccess<infra::ByteRange> flashStorage, infra::ByteRange ramStorage)
        : flashStorage(flashStorage)
        , ramStorage(ramStorage)
    {
        infra::Copy(*flashStorage, ramStorage);
    }

    void BondBlobPersistence::Update()
    {
        infra::Copy(ramStorage, *flashStorage);
        flashStorage.Write();
    }
}
