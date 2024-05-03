#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/MemoryRange.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "protobuf/echo/Echo.hpp"
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
}

namespace application
{
    class PeerServiceDiscovererEcho;

    class PeerServiceDiscoveryObserver
        : public infra::SingleObserver<PeerServiceDiscoveryObserver, PeerServiceDiscovererEcho>
    {
    public:
        using infra::SingleObserver<PeerServiceDiscoveryObserver, PeerServiceDiscovererEcho>::SingleObserver;

        virtual void ServiceDiscoveryStarted() = 0;
        virtual void ServiceDiscoveryComplete(infra::MemoryRange<uint32_t> services) = 0;
    };

    class PeerServiceDiscovererEcho
        : public service_discovery::ServiceDiscoveryProxy
        , public service_discovery::ServiceDiscoveryResponse
        , public infra::Subject<PeerServiceDiscoveryObserver>
    {
    public:
        explicit PeerServiceDiscovererEcho(services::Echo& echo)
            : service_discovery::ServiceDiscoveryProxy(echo)
            , service_discovery::ServiceDiscoveryResponse(echo)
        {
            infra::EventDispatcher::Instance().Schedule([this]()
                {
                    Initialize();
                });
        }

        void NoServiceSupported() override
        {
            NotifyObservers([this](auto& observer)
                {
                    observer.ServiceDiscoveryComplete(services.range());
                });
        }

        void FirstServiceSupported(uint32_t id) override
        {
            services.push_back(id);
            MethodDone();
            RequestSend([this]
                {
                    FindFirstServiceInRange(services.back(), std::numeric_limits<uint32_t>::max());
                });
        }

        void ServicesChanged() override
        {}

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
            NotifyObservers([this](auto& observer)
                {
                    observer.ServiceDiscoveryStarted();
                });

            RequestSend([this]
                {
                    FindFirstServiceInRange(0, std::numeric_limits<uint32_t>::max());
                });
        }

    private:
        infra::BoundedVector<uint32_t>::WithMaxSize<2> services;
    };
}

namespace
{
    class PeerServiceDiscoveryObserverMock
        : public application::PeerServiceDiscoveryObserver
    {
    public:
        using application::PeerServiceDiscoveryObserver::PeerServiceDiscoveryObserver;

        MOCK_METHOD(void, ServiceDiscoveryStarted, ());
        MOCK_METHOD(void, ServiceDiscoveryComplete, (infra::MemoryRange<uint32_t>));
    };
}

class PeerServiceDiscovererTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    services::MethodSerializerFactory::ForServices<service_discovery::ServiceDiscovery, service_discovery::ServiceDiscoveryResponse>::AndProxies<service_discovery::ServiceDiscoveryProxy, service_discovery::ServiceDiscoveryResponseProxy> serializerFactory;
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
        EXPECT_CALL(observer, ServiceDiscoveryStarted());
        EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(0, std::numeric_limits<uint32_t>::max()));

        ExecuteAllActions();
    }

    void QueryServices(infra::MemoryRange<uint32_t> services)
    {
        for ( auto service : services )
        {    
            EXPECT_CALL(serviceDiscovery, FindFirstServiceInRangeMock(service, std::numeric_limits<uint32_t>::max()));
            ServiceSupported(service);
        }
    }
};

TEST_F(PeerServiceDiscovererTest, construct_schedules_discovery_start)
{
    StartInitialServiceDiscovery();
}

TEST_F(PeerServiceDiscovererTest, immediate_NoServiceSupported_ends_discovery_with_no_services)
{
    StartInitialServiceDiscovery();

    infra::MemoryRange<uint32_t> services;
    ServiceDiscoveryComplete(services);
}

TEST_F(PeerServiceDiscovererTest, NoServiceSupported_ends_discovery_with_single_service)
{
    StartInitialServiceDiscovery();

    std::array<uint32_t, 1> services{ 5 };
    QueryServices(services);

    ServiceDiscoveryComplete(services);
}

TEST_F(PeerServiceDiscovererTest, NoServiceSupported_ends_discovery_with_two_services)
{
    StartInitialServiceDiscovery();

    std::array<uint32_t, 2> services{ 5, 10 };
    QueryServices(services);

    ServiceDiscoveryComplete(services);
}
