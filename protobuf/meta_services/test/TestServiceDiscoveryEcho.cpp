#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/util/Optional.hpp"
#include "protobuf/echo/Echo.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <sys/types.h>

namespace
{
    class ServiceDiscoveryResponseMock
        : public service_discovery::ServiceDiscoveryResponse
    {
    public:
        using service_discovery::ServiceDiscoveryResponse::ServiceDiscoveryResponse;
        virtual ~ServiceDiscoveryResponseMock() = default;

        void FirstServiceSupported(uint32_t value) override
        {
            FirstServiceSupportedMock(value);
            MethodDone();
        }

        void NoServiceSupported() override
        {
            NoServiceSupportedMock();
            MethodDone();
        }

        void ServicesChanged() override
        {
            ServicesChangedMock();
            MethodDone();
        }

        MOCK_METHOD(void, FirstServiceSupportedMock, (uint32_t));
        MOCK_METHOD(void, NoServiceSupportedMock, ());
        MOCK_METHOD(void, ServicesChangedMock, ());
    };

    class ServiceStubWithCustomServiceId
        : public services::ServiceStub
    {
    public:
        ServiceStubWithCustomServiceId(services::Echo& echo, uint32_t serviceId)
            : services::ServiceStub(echo)
            , serviceId(serviceId)
        {}

        virtual ~ServiceStubWithCustomServiceId() = default;

        bool AcceptsService(uint32_t serviceId) const override
        {
            return this->serviceId == serviceId;
        }

    private:
        const uint32_t serviceId;
    };
};

class ServiceDiscoveryTest
    : public testing::Test
{
public:
    services::MethodSerializerFactory::ForServices<service_discovery::ServiceDiscovery, service_discovery::ServiceDiscoveryResponse>::AndProxies<service_discovery::ServiceDiscoveryProxy> serializerFactory;
    application::EchoSingleLoopback echo{ serializerFactory };
    service_discovery::ServiceDiscoveryProxy proxy{ echo };
    testing::StrictMock<ServiceDiscoveryResponseMock> serviceDiscoveryResponse{ echo };

    application::ServiceDiscoveryEcho serviceDiscoveryEcho{ echo };
};

TEST_F(ServiceDiscoveryTest, return_no_service)
{
    ServiceStubWithCustomServiceId serviceMock{ serviceDiscoveryEcho, 5 };

    EXPECT_CALL(serviceDiscoveryResponse, NoServiceSupportedMock);

    proxy.RequestSend([this]
        {
            proxy.FindFirstServiceInRange(0, 4);
        });
}

TEST_F(ServiceDiscoveryTest, return_service)
{
    ServiceStubWithCustomServiceId serviceMock{ serviceDiscoveryEcho, 5 };

    EXPECT_CALL(serviceDiscoveryResponse, FirstServiceSupportedMock(5));

    proxy.RequestSend([this]
        {
            proxy.FindFirstServiceInRange(0, 15);
        });
}

TEST_F(ServiceDiscoveryTest, return_service_with_lowest_id)
{
    ServiceStubWithCustomServiceId serviceMock{ serviceDiscoveryEcho, 5 };
    ServiceStubWithCustomServiceId serviceMock2{ serviceDiscoveryEcho, 6 };

    EXPECT_CALL(serviceDiscoveryResponse, FirstServiceSupportedMock(5));

    proxy.RequestSend([this]
        {
            proxy.FindFirstServiceInRange(0, 15);
        });
}

TEST_F(ServiceDiscoveryTest, DISABLED_notify_service_change)
{
    proxy.RequestSend([this]
        {
            proxy.NotifyServiceChanges(true);
        });

    EXPECT_CALL(serviceDiscoveryResponse, ServicesChangedMock());

    ServiceStubWithCustomServiceId serviceMock{ serviceDiscoveryEcho, 5 };
}

TEST_F(ServiceDiscoveryTest, find_own_service_id)
{
    EXPECT_CALL(serviceDiscoveryResponse, FirstServiceSupportedMock(service_discovery::ServiceDiscovery::serviceId));

    proxy.RequestSend([this]
        {
            proxy.FindFirstServiceInRange(service_discovery::ServiceDiscovery::serviceId, service_discovery::ServiceDiscovery::serviceId);
        });
}
