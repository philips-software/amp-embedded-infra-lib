#ifndef SERVICES_PERSISTING_BOND_STORAGE_HPP
#define SERVICES_PERSISTING_BOND_STORAGE_HPP

#include "services/util/ConfigurationStore.hpp"

namespace services
{
    class PersistingBondStorage
    {
    public:
        PersistingBondStorage(services::ConfigurationStoreAccess<infra::BoundedVector<uint8_t>> flashStorage, infra::ByteRange ramStorage);

        void Update();

    private:
        services::ConfigurationStoreAccess<infra::BoundedVector<uint8_t>> flashStorage;
        infra::ByteRange ramStorage;
    };
}

#endif
