#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/MemoryRange.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "protobuf/meta_services/PeerServiceDiscoverer.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include <array>
#include <cstdint>
#include <limits>

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

    class PeerServiceDiscoveryObserverMock
        : public application::PeerServiceDiscoveryObserver
    {
    public:
        using application::PeerServiceDiscoveryObserver::PeerServiceDiscoveryObserver;

        MOCK_METHOD(void, ServiceDiscoveryComplete, (infra::MemoryRange<uint32_t>), (override));
    };
}

class PeerServiceDiscovererTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    services::MethodSerializerFactory::ForServices<service_discovery::ServiceDiscovery, 
        service_discovery::ServiceDiscoveryResponse>::AndProxies<service_discovery::ServiceDiscoveryProxy, 
        service_discovery::ServiceDiscoveryResponseProxy> serializerFactory;
    
    application::EchoSingleLoopback echo{ serializerFactory };
    
    service_discovery::ServiceDiscoveryResponseProxy serviceDiscoveryResponse{ echo };
    testing::StrictMock<ServiceDiscoveryMock> serviceDiscovery{ echo };

    application::PeerServiceDiscovererEcho discoverer{echo};
    testing::StrictMock<PeerServiceDiscoveryObserverMock> observer{discoverer};

    void NoServiceSupported()
    {
        serviceDiscoveryResponse.RequestSend([this]
            {
                serviceDiscoveryResponse.NoServiceSupported();
            });
    }

    void ServiceDiscoveryComplete(infra::MemoryRange<uint32_t> services)
    {
        EXPECT_CALL(observer, ServiceDiscoveryComplete(infra::ByteRangeContentsEqual(services)));
        NoServiceSupported();
    }

    void ServiceSupported(uint32_t id)
    {
        serviceDiscoveryResponse.RequestSend([this, id]
            {
                serviceDiscoveryResponse.FirstServiceSupported(id);
            });
    }

    void StartInitialServiceDiscovery()
    {
        EXPECT_CALL(serviceDiscovery, NotifyServiceChangesMock(true));
        EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(0, std::numeric_limits<uint32_t>::max()));

        ExecuteAllActions();
    }

    void QueryServices(infra::MemoryRange<uint32_t> services)
    {
        for (auto service : services)
        {    
            EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(service, std::numeric_limits<uint32_t>::max()));
            ServiceSupported(service);
        }
    }

    void OneCompleteRoundOfServiceDiscovery(infra::MemoryRange<uint32_t> services)
    {
        StartInitialServiceDiscovery();

        QueryServices(services);

        ServiceDiscoveryComplete(services);
    }
};

TEST_F(PeerServiceDiscovererTest, construct_schedules_discovery_start)
{
    StartInitialServiceDiscovery();
}

TEST_F(PeerServiceDiscovererTest, immediate_NoServiceSupported_ends_discovery_with_no_services)
{
    infra::MemoryRange<uint32_t> services;
    OneCompleteRoundOfServiceDiscovery(services);
}

TEST_F(PeerServiceDiscovererTest, NoServiceSupported_ends_discovery_with_single_service)
{
    std::array<uint32_t, 1> services{ 5 };
    OneCompleteRoundOfServiceDiscovery(services);
}

TEST_F(PeerServiceDiscovererTest, NoServiceSupported_ends_discovery_with_two_services)
{
    std::array<uint32_t, 2> services{ 5, 10 };
    OneCompleteRoundOfServiceDiscovery(services);
}

TEST_F(PeerServiceDiscovererTest, ServicesChanged_triggers_rediscovery)
{
    std::array<uint32_t, 2> services{ 5, 10 };
    OneCompleteRoundOfServiceDiscovery(services);

    StartInitialServiceDiscovery();

    serviceDiscoveryResponse.RequestSend([this]
    {
        serviceDiscoveryResponse.ServicesChanged(0, 0);
    });

    std::array<uint32_t, 2> servicesUpdated{ 5, 20 };
    QueryServices(servicesUpdated);

    ServiceDiscoveryComplete(servicesUpdated);
}

