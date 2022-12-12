#ifndef SERVICES_CONFIGURATION_STORE_MOCK_HPP
#define SERVICES_CONFIGURATION_STORE_MOCK_HPP

#include "services/util/ConfigurationStore.hpp"
#include "gmock/gmock.h"

namespace services
{
    class ConfigurationStoreInterfaceMock
        : public ConfigurationStoreInterface
    {
    public:
        MOCK_METHOD0(Write, uint32_t());
        MOCK_METHOD0(Unlocked, void());
    };
}

#endif
