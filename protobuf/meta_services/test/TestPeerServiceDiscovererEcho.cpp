#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <limits>
#include "protobuf/echo/Echo.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"

namespace
{
    class ServiceDiscoveryMock
        : public service_discovery::ServiceDiscovery
    {
    public:
        using service_discovery::ServiceDiscovery::ServiceDiscovery;

        void FindFirstServiceInRange(uint32_t startServiceId, uint32_t endServiceId) override
        {
            FindFirstServiceInRangeMock(startServiceId, endServiceId);
            MethodDone();
        }
        
        void NotifyServiceChanges(bool value) override
        {
            NotifyServiceChangesMock(value);
            MethodDone();
        }

        MOCK_METHOD(void, FindFirstServiceInRangeMock, (uint32_t, uint32_t));
        MOCK_METHOD(void, NotifyServiceChangesMock, (bool));
    };
}

namespace application {

    class PeerServiceDiscovererEcho
        : public service_discovery::ServiceDiscoveryProxy
    {
    public:
        PeerServiceDiscovererEcho(services::Echo& echo)
            : service_discovery::ServiceDiscoveryProxy(echo)
        {
            
            Initialize();
        }

    private:
        void Initialize()
        {
            RequestSend([this]
                {
                    NotifyServiceChanges(true);
                    StartDiscovery();
                });
        }

        void StartDiscovery()
        {
            RequestSend([this]
            {
                FindFirstServiceInRange(0, std::numeric_limits<uint32_t>::max());
            });
        }
    };
}

class PeerServiceDiscovererTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::ForServices<service_discovery::ServiceDiscovery, service_discovery::ServiceDiscoveryResponse>::AndProxies<service_discovery::ServiceDiscoveryProxy, service_discovery::ServiceDiscoveryResponseProxy> serializerFactory;
    application::EchoSingleLoopback echo{ serializerFactory };
    
    service_discovery::ServiceDiscoveryResponseProxy serviceDiscoveryResponse{ echo };
    testing::StrictMock<ServiceDiscoveryMock> serviceDiscovery{ echo };

    infra::Execute turnOnNotifyServiceChanges{[this]
        {
            EXPECT_CALL(serviceDiscovery, NotifyServiceChangesMock(true));
            EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(0, std::numeric_limits<uint32_t>::max()));
        }};
    application::PeerServiceDiscovererEcho discoverer{echo};
};

TEST_F(PeerServiceDiscovererTest, construct)
{}

// TEST_F(PeerServiceDiscovererTest, )
// {
// }

