#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/MemoryRange.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "protobuf/echo/test_doubles/EchoSingleLoopback.hpp"
#include "protobuf/meta_services/PeerServiceDiscoverer.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <array>
#include <cstdint>
#include <limits>
#include <tuple>

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

        MOCK_METHOD(void, ServicesDiscovered, (infra::MemoryRange<uint32_t>), (override));
    };
}

class PeerServiceDiscovererTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    services::MethodSerializerFactory::ForServices<service_discovery::ServiceDiscovery,
        service_discovery::ServiceDiscoveryResponse>::AndProxies<service_discovery::ServiceDiscoveryProxy,
        service_discovery::ServiceDiscoveryResponseProxy>
        serializerFactory;

    application::EchoSingleLoopback echo{ serializerFactory };

    service_discovery::ServiceDiscoveryResponseProxy serviceDiscoveryResponse{ echo };
    testing::StrictMock<ServiceDiscoveryMock> serviceDiscovery{ echo };

    infra::Execute execute{ [this]()
        {
            StartInitialServiceDiscovery();
        } };
    application::PeerServiceDiscovererEcho discoverer{ echo };
    testing::StrictMock<PeerServiceDiscoveryObserverMock> observer{ discoverer };

    void NoServiceSupported()
    {
        serviceDiscoveryResponse.RequestSend([this]
            {
                serviceDiscoveryResponse.NoServiceSupported();
            });
    }

    void ServicesDiscovered(infra::MemoryRange<uint32_t> services)
    {
        EXPECT_CALL(observer, ServicesDiscovered(infra::ByteRangeContentsEqual(services)));
        NoServiceSupported();
    }

    void ServiceSupported(uint32_t id)
    {
        serviceDiscoveryResponse.RequestSend([this, id]
            {
                serviceDiscoveryResponse.FirstServiceSupported(id);
            });
    }

    void ServicesChanged(uint32_t startServiceId, uint32_t endServiceId)
    {
        serviceDiscoveryResponse.RequestSend([this, startServiceId, endServiceId]
            {
                serviceDiscoveryResponse.ServicesChanged(startServiceId, endServiceId);
            });
    }

    void StartInitialServiceDiscovery()
    {
        EXPECT_CALL(serviceDiscovery, NotifyServiceChangesMock(true));
        EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(0, std::numeric_limits<uint32_t>::max()));
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
        QueryServices(services);
        ServicesDiscovered(services);
    }

    using ServicesList = infra::MemoryRange<uint32_t>;

    std::tuple<std::set<uint32_t>, std::set<uint32_t>> FindUpdatedSet(ServicesList oldServiceList, ServicesList newServiceList)
    {
        std::set<uint32_t> oldSet(oldServiceList.begin(), oldServiceList.end());
        std::set<uint32_t> newSet(newServiceList.begin(), newServiceList.end());

        std::set<uint32_t> addedServices;
        std::set<uint32_t> removedServices;

        std::set_difference(newSet.begin(), newSet.end(), oldSet.begin(), oldSet.end(), std::inserter(addedServices, addedServices.end()));
        std::set_difference(oldSet.begin(), oldSet.end(), newSet.begin(), newSet.end(), std::inserter(removedServices, removedServices.end()));

        return std::make_tuple(addedServices, removedServices);
    }

    auto FindUpdatedRange(std::set<uint32_t> range1, std::set<uint32_t> range2)
    {
        if (range1.empty())
            return std::make_tuple(*range2.begin(), *range2.rbegin());
        else if (range2.empty())
            return std::make_tuple(*range1.begin(), *range1.rbegin());
        else
            return std::make_tuple(std::min(*range1.begin(), *range2.begin()), std::max(*range1.rbegin(), *range2.rbegin()));
    }

    void UpdateServices(ServicesList oldServiceList, ServicesList newServiceList)
    {
        auto [addedServices, removedServices] = FindUpdatedSet(oldServiceList, newServiceList);
        
        auto updatedRange = FindUpdatedRange(addedServices, removedServices);

    }
};

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

    EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(0, 0));

    ServicesChanged(0, 0);

    EXPECT_CALL(observer, ServicesDiscovered(infra::ByteRangeContentsEqual(infra::MakeRange(services))));
    NoServiceSupported();

    EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(5, 5));

    ServicesChanged(5, 5);

    std::array<uint32_t, 1> servicesUpdated{ 10 };
    EXPECT_CALL(observer, ServicesDiscovered(infra::ByteRangeContentsEqual(infra::MakeRange(servicesUpdated))));
    NoServiceSupported();

    std::vector<uint32_t> oldServices{ 1, 3, 5 };
    std::vector<uint32_t> newServices{ 5, 6, 3, 4 };
    UpdateServices(infra::MakeRange(oldServices), infra::MakeRange(newServices));
}
