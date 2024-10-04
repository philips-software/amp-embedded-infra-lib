#ifndef PROTOBUF_ECHO_PEER_SERVICE_DISCOVERER_HPP
#define PROTOBUF_ECHO_PEER_SERVICE_DISCOVERER_HPP

#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/syntax/CppFormatter.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/MemoryRange.hpp"
#include "infra/util/Observer.hpp"
#include "protobuf/echo/Echo.hpp"
#include <cstdint>
#include <tuple>

namespace application
{
    class PeerServiceDiscovererEcho;

    class PeerServiceDiscoveryObserver
        : public infra::SingleObserver<PeerServiceDiscoveryObserver, PeerServiceDiscovererEcho>
    {
    public:
        using infra::SingleObserver<PeerServiceDiscoveryObserver, PeerServiceDiscovererEcho>::SingleObserver;

        virtual void ServicesDiscovered(infra::MemoryRange<uint32_t> services) = 0;
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
        void ServicesChanged(uint32_t startServiceId, uint32_t endServiceId) override;

    private:
        using ServiceRange = std::tuple<uint32_t, uint32_t>;
        static constexpr ServiceRange serviceRangeMax = std::make_tuple(0, std::numeric_limits<uint32_t>::max());

    private:
        void DiscoverPeerServices();
        void StartDiscovery(ServiceRange range = serviceRangeMax);

    private:
        infra::BoundedVector<uint32_t>::WithMaxSize<2> services;
    };
}

#endif
