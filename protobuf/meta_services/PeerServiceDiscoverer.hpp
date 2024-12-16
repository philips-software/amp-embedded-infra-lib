#ifndef PROTOBUF_ECHO_PEER_SERVICE_DISCOVERER_HPP
#define PROTOBUF_ECHO_PEER_SERVICE_DISCOVERER_HPP

#include "generated/echo/ServiceDiscovery.pb.hpp"
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
        : public infra::Observer<PeerServiceDiscoveryObserver, PeerServiceDiscovererEcho>
    {
    public:
        using infra::Observer<PeerServiceDiscoveryObserver, PeerServiceDiscovererEcho>::Observer;

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
        void StartDiscovery();
        void ClearUpdatedServices();
        uint32_t SearchRangeBegin() const;
        uint32_t SearchRangeEnd() const;

    private:
        infra::BoundedVector<uint32_t>::WithMaxSize<100> services;
        ServiceRange searchRange;
    };
}

#endif
