#ifndef SERVICES_BOND_BLOB_PERSISTENCE_HPP
#define SERVICES_BOND_BLOB_PERSISTENCE_HPP

#include "services/util/ConfigurationStore.hpp"

namespace services
{
    class BondBlobPersistence
    {
    public:
        BondBlobPersistence(services::ConfigurationStoreAccess<infra::ByteRange> flashStorage, infra::ByteRange ramStorage);

        void Update();

    private:
        services::ConfigurationStoreAccess<infra::ByteRange> flashStorage;
        infra::ByteRange ramStorage;
    };
}

#endif
