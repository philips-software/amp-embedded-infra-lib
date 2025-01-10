#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/Optional.hpp"
#include "protobuf/echo/Echo.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <sys/types.h>
#include <tuple>

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

        void ServicesChanged(uint32_t startServiceId, uint32_t endServiceId) override
        {
            ServicesChangedMock(startServiceId, endServiceId);
            MethodDone();
        }

        MOCK_METHOD(void, FirstServiceSupportedMock, (uint32_t serviceId));
        MOCK_METHOD(void, NoServiceSupportedMock, ());
        MOCK_METHOD(void, ServicesChangedMock, (uint32_t startServiceId, uint32_t endServiceId));
    };
};

class ServiceDiscoveryTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    services::MethodSerializerFactory::ForServices<service_discovery::ServiceDiscovery, service_discovery::ServiceDiscoveryResponse>::AndProxies<service_discovery::ServiceDiscoveryProxy> serializerFactory;
    application::EchoSingleLoopback echo{ serializerFactory };
    service_discovery::ServiceDiscoveryProxy proxy{ echo };
    testing::StrictMock<ServiceDiscoveryResponseMock> serviceDiscoveryResponse{ echo };
    application::ServiceDiscoveryEcho serviceDiscoveryEcho{ echo };

    void FindFirstServiceInRange(uint32_t startServiceId, uint32_t endServiceId)
    {
        auto serviceRange = std::tie(startServiceId, endServiceId);
        proxy.RequestSend([this, &serviceRange]
            {
                proxy.FindFirstServiceInRange(std::get<0>(serviceRange), std::get<1>(serviceRange));
            });
    }

    void NotifyServiceChanges(bool allow)
    {
        proxy.RequestSend([this, allow]
            {
                proxy.NotifyServiceChanges(allow);
            });
    }
};

TEST_F(ServiceDiscoveryTest, return_no_service)
{
    services::ServiceStub service5{ serviceDiscoveryEcho, 5 };

    FindFirstServiceInRange(0, 4);

    EXPECT_CALL(serviceDiscoveryResponse, NoServiceSupportedMock);
    ExecuteAllActions();
}

TEST_F(ServiceDiscoveryTest, return_service)
{
    services::ServiceStub service5{ serviceDiscoveryEcho, 5 };

    FindFirstServiceInRange(0, 15);

    EXPECT_CALL(serviceDiscoveryResponse, FirstServiceSupportedMock(5));
    ExecuteAllActions();
}

TEST_F(ServiceDiscoveryTest, return_service_with_lowest_id)
{
    services::ServiceStub service5{ serviceDiscoveryEcho, 5 };
    services::ServiceStub service6{ serviceDiscoveryEcho, 6 };

    FindFirstServiceInRange(0, 15);

    EXPECT_CALL(serviceDiscoveryResponse, FirstServiceSupportedMock(5));
    ExecuteAllActions();
}

TEST_F(ServiceDiscoveryTest, notify_service_change)
{
    NotifyServiceChanges(true);

    infra::Optional<services::ServiceStub> service5(infra::inPlace, serviceDiscoveryEcho, 5);

    EXPECT_CALL(serviceDiscoveryResponse, ServicesChangedMock(5, 5));
    ForwardTime(std::chrono::milliseconds(500));

    service5 = infra::none;

    EXPECT_CALL(serviceDiscoveryResponse, ServicesChangedMock(5, 5));
    ForwardTime(std::chrono::milliseconds(500));

    NotifyServiceChanges(false);

    service5.Emplace(serviceDiscoveryEcho, 5);
    ForwardTime(std::chrono::milliseconds(500));
}

TEST_F(ServiceDiscoveryTest, notify_simultaneous_service_changes)
{
    NotifyServiceChanges(true);

    services::ServiceStub service5(serviceDiscoveryEcho, 5);
    services::ServiceStub service15(serviceDiscoveryEcho, 15);

    EXPECT_CALL(serviceDiscoveryResponse, ServicesChangedMock(5, 15));
    ForwardTime(std::chrono::milliseconds(500));

    services::ServiceStub service7(serviceDiscoveryEcho, 7);
    services::ServiceStub service12(serviceDiscoveryEcho, 12);

    EXPECT_CALL(serviceDiscoveryResponse, ServicesChangedMock(7, 12));
    ForwardTime(std::chrono::milliseconds(500));

    NotifyServiceChanges(false);
}

TEST_F(ServiceDiscoveryTest, notify_service_changes_only_after_NotifyServiceChanges)
{
    NotifyServiceChanges(true);
    services::ServiceStub service5(serviceDiscoveryEcho, 5);

    NotifyServiceChanges(false);
    services::ServiceStub service15(serviceDiscoveryEcho, 15);

    NotifyServiceChanges(true);
    services::ServiceStub service25(serviceDiscoveryEcho, 25);

    EXPECT_CALL(serviceDiscoveryResponse, ServicesChangedMock(25, 25));
    ForwardTime(std::chrono::milliseconds(500));

    NotifyServiceChanges(false);
}

TEST_F(ServiceDiscoveryTest, find_own_service_id)
{
    FindFirstServiceInRange(service_discovery::ServiceDiscovery::serviceId, service_discovery::ServiceDiscovery::serviceId);

    EXPECT_CALL(serviceDiscoveryResponse, FirstServiceSupportedMock(service_discovery::ServiceDiscovery::serviceId));
    ExecuteAllActions();
}

TEST_F(ServiceDiscoveryTest, start_proxy_service_method)
{
    services::ServiceStub service1{ serviceDiscoveryEcho, 1 };
    services::ServiceStubProxy service1Proxy{ echo, 1 };

    services::ServiceStub service2{ serviceDiscoveryEcho, 2 };
    services::ServiceStubProxy service2Proxy{ echo, 2 };

    EXPECT_CALL(service1, Method(11)).WillOnce(testing::InvokeWithoutArgs([&service1]
        {
            service1.MethodDone();
        }));

    service1Proxy.RequestSend([&service1Proxy]
        {
            service1Proxy.Method(11);
        });

    EXPECT_CALL(service2, Method(22));
    service2Proxy.RequestSend([&service2Proxy]
        {
            service2Proxy.Method(22);
        });
}

TEST_F(ServiceDiscoveryTest, forward_methods_only_to_first_matching_proxy_service)
{
    services::ServiceStub service1{ serviceDiscoveryEcho, 1 };
    services::ServiceStub service1Copy{ serviceDiscoveryEcho, 1 };
    services::ServiceStubProxy service1Proxy{ echo, 1 };

    EXPECT_CALL(service1, Method(11)).WillOnce(testing::InvokeWithoutArgs([&service1]
        {
            service1.MethodDone();
        }));

    service1Proxy.RequestSend([&service1Proxy]
        {
            service1Proxy.Method(11);
        });
}
