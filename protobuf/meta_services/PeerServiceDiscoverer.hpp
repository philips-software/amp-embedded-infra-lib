#ifndef PROTOBUF_ECHO_PEER_SERVICE_DISCOVERER_HPP
#define PROTOBUF_ECHO_PEER_SERVICE_DISCOVERER_HPP

#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/MemoryRange.hpp"
#include "infra/util/Observer.hpp"
#include "protobuf/echo/Echo.hpp"

#include <cstdint>

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
        explicit PeerServiceDiscovererEcho(services::Echo& echo);

        void NoServiceSupported() override;
        void FirstServiceSupported(uint32_t id) override;
        void ServicesChanged() override;

    private:
        void Initialize();
        void StartDiscovery();
    private:
        infra::BoundedVector<uint32_t>::WithMaxSize<2> services;
    };
}

#endif
