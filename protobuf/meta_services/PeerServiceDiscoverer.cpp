#include "protobuf/meta_services/PeerServiceDiscoverer.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/Function.hpp"
#include <cstdint>
#include <limits>

namespace application
{
    PeerServiceDiscovererEcho::PeerServiceDiscovererEcho(services::Echo& echo)
        : service_discovery::ServiceDiscoveryProxy(echo)
        , service_discovery::ServiceDiscoveryResponse(echo)
    {
        DiscoverPeerServices();
    }

    void PeerServiceDiscovererEcho::NoServiceSupported()
    {
        NotifyObservers([this](auto& observer)
            {
                observer.ServicesDiscovered(services.range());
            });

        MethodDone();
    }

    void PeerServiceDiscovererEcho::FirstServiceSupported(uint32_t id)
    {
        services.push_back(id);
        RequestSend([this]
            {
                FindFirstServiceInRange(services.back(), std::numeric_limits<uint32_t>::max());
            });

        MethodDone();
    }

    void PeerServiceDiscovererEcho::ServicesChanged(uint32_t startServiceId, uint32_t endServiceId)
    {
        StartDiscovery(ServiceRange{ startServiceId, endServiceId });

        MethodDone();
    }

    void PeerServiceDiscovererEcho::DiscoverPeerServices()
    {
        RequestSend([this]
            {
                NotifyServiceChanges(true);
                StartDiscovery();
            });
    }

    void PeerServiceDiscovererEcho::StartDiscovery(ServiceRange range)
    {
        services.erase(std::remove_if(services.begin(), services.end(), [range](uint32_t serviceId)
                           {
                               return serviceId >= std::get<0>(range) && serviceId <= std::get<1>(range);
                           }),
            services.end());

        RequestSend([this, range]
            {
                FindFirstServiceInRange(std::get<0>(range), std::get<1>(range));
            });
    }
}
