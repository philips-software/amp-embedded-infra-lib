#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "services/echo_console/ConsoleService.hpp"
#include "gtest/gtest.h"

class ConsoleServiceTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::ForServices<service_discovery::ServiceDiscovery,
        service_discovery::ServiceDiscoveryResponse>::AndProxies<service_discovery::ServiceDiscoveryProxy,
        service_discovery::ServiceDiscoveryResponseProxy>
        serializerFactory;

    application::EchoSingleLoopback echo{};
};
