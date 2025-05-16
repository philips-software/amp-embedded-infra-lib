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
#include <vector>

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

    void ServicesDiscovered(const std::vector<uint32_t>& services)
    {
        EXPECT_CALL(observer, ServicesDiscovered(infra::ContentsEqual(services)));
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

    void QueryServices(const std::vector<uint32_t>& services, uint32_t searchEnd = std::numeric_limits<uint32_t>::max())
    {
        auto orderedServices = services;
        std::sort(orderedServices.begin(), orderedServices.end());

        for (auto service : orderedServices)
        {
            if (service + 1 <= searchEnd)
                EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(service + 1, searchEnd));
            ServiceSupported(service);
        }

        if (services.empty() || services.back() != searchEnd)
            NoServiceSupported();
    }

    void OneCompleteRoundOfServiceDiscovery(const std::vector<uint32_t>& services)
    {
        EXPECT_CALL(observer, ServicesDiscovered(infra::ContentsEqual(services)));
        QueryServices(services);
    }

    auto FindAddedAndRemovedServices(const std::vector<uint32_t>& oldServiceList, const std::vector<uint32_t>& newServiceList)
    {
        std::set<uint32_t> oldSet(oldServiceList.begin(), oldServiceList.end());
        std::set<uint32_t> newSet(newServiceList.begin(), newServiceList.end());

        std::set<uint32_t> addedServices;
        std::set<uint32_t> removedServices;

        std::set_difference(newSet.begin(), newSet.end(), oldSet.begin(), oldSet.end(), std::inserter(addedServices, addedServices.end()));
        std::set_difference(oldSet.begin(), oldSet.end(), newSet.begin(), newSet.end(), std::inserter(removedServices, removedServices.end()));

        std::vector<uint32_t> addedServicesVector(addedServices.begin(), addedServices.end());
        std::vector<uint32_t> removedServicesVector(removedServices.begin(), removedServices.end());

        return std::make_tuple(addedServicesVector, removedServicesVector);
    }

    auto FindMaximalRange(const std::vector<uint32_t>& range1, const std::vector<uint32_t>& range2)
    {
        if (range1.empty())
            return std::make_tuple(*range2.begin(), *range2.rbegin());
        else if (range2.empty())
            return std::make_tuple(*range1.begin(), *range1.rbegin());
        else
            return std::make_tuple(std::min(*range1.begin(), *range2.begin()), std::max(*range1.rbegin(), *range2.rbegin()));
    }

    auto FindServicesInRange(const std::vector<uint32_t>& newServiceList, const std::tuple<uint32_t, uint32_t>& updatedRange)
    {
        const auto searchBegin = std::get<0>(updatedRange);
        const auto searchEnd = std::get<1>(updatedRange);

        std::vector<uint32_t> filteredServices;
        for (const auto& service : newServiceList)
        {
            if (service >= searchBegin && service <= searchEnd)
            {
                filteredServices.push_back(service);
            }
        }

        std::sort(filteredServices.begin(), filteredServices.end());
        return filteredServices;
    }

    void UpdateServices(const std::vector<uint32_t>& oldServiceList, const std::vector<uint32_t>& newServiceList)
    {
        auto [addedServices, removedServices] = FindAddedAndRemovedServices(oldServiceList, newServiceList);

        auto updatedRange = FindMaximalRange(addedServices, removedServices);
        auto searchBegin = std::get<0>(updatedRange);
        auto searchEnd = std::get<1>(updatedRange);

        EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(searchBegin, searchEnd));
        ServicesChanged(searchBegin, searchEnd);

        auto filteredServices = FindServicesInRange(newServiceList, updatedRange);
        EXPECT_CALL(observer, ServicesDiscovered(infra::ContentsEqual(newServiceList)));
        QueryServices(filteredServices, searchEnd);
    }
};

TEST_F(PeerServiceDiscovererTest, immediate_NoServiceSupported_ends_discovery_with_no_services)
{
    const std::vector<uint32_t> services;
    OneCompleteRoundOfServiceDiscovery(services);
}

TEST_F(PeerServiceDiscovererTest, NoServiceSupported_ends_discovery_with_single_service)
{
    std::vector<uint32_t> services{ 5 };
    OneCompleteRoundOfServiceDiscovery(services);
}

TEST_F(PeerServiceDiscovererTest, NoServiceSupported_ends_discovery_with_two_services)
{
    std::vector<uint32_t> services{ 5, 10 };
    OneCompleteRoundOfServiceDiscovery(services);
}

TEST_F(PeerServiceDiscovererTest, ServicesChanged_triggers_rediscovery)
{
    std::vector<uint32_t> services{ 5, 10 };
    OneCompleteRoundOfServiceDiscovery(services);

    EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(0, 0));

    ServicesChanged(0, 0);

    EXPECT_CALL(observer, ServicesDiscovered(infra::ContentsEqual(services)));
    NoServiceSupported();

    EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(5, 5));
    ServicesChanged(5, 5);

    std::vector<uint32_t> servicesUpdated{ 10 };
    EXPECT_CALL(observer, ServicesDiscovered(infra::ContentsEqual(servicesUpdated)));
    NoServiceSupported();

    EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(10, 10));
    ServicesChanged(10, 10);

    std::vector<uint32_t> servicesUpdated2;
    EXPECT_CALL(observer, ServicesDiscovered(infra::ContentsEqual(servicesUpdated2)));
    NoServiceSupported();
}

TEST_F(PeerServiceDiscovererTest, ServicesChanged_triggers_rediscovery_2)
{
    std::vector<uint32_t> services{ 1, 3, 5 };
    OneCompleteRoundOfServiceDiscovery(services);

    std::vector<uint32_t> newServices{ 3, 4, 5, 6 };
    UpdateServices(services, newServices);
}
