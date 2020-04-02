#ifndef SERVICES_CONFIGURATION_STORE_MOCK_HPP
#define SERVICES_CONFIGURATION_STORE_MOCK_HPP

#include "gmock/gmock.h"
#include "services/util/ConfigurationStore.hpp"

namespace services
{
    class ConfigurationStoreInterfaceMock
        : public ConfigurationStoreInterface
    {
    public:
        MOCK_METHOD0(Write, uint32_t());
    };
}

#endif
