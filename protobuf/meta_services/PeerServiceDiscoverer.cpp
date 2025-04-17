#include "protobuf/meta_services/PeerServiceDiscoverer.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/Function.hpp"
#include "protobuf/meta_services/PeerServiceDiscoverer.hpp"
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

        if (id == SearchRangeEnd())
            NotifyObservers([this](auto& observer)
                {
                    observer.ServicesDiscovered(services.range());
                });
        else
            RequestSend([this]
                {
                    FindFirstServiceInRange(services.back() + 1, SearchRangeEnd());
                });

        MethodDone();
    }

    void PeerServiceDiscovererEcho::ServicesChanged(uint32_t startServiceId, uint32_t endServiceId)
    {
        searchRange = ServiceRange{ startServiceId, endServiceId };

        StartDiscovery();

        MethodDone();
    }

    void PeerServiceDiscovererEcho::DiscoverPeerServices()
    {
        searchRange = serviceRangeMax;

        RequestSend([this]
            {
                NotifyServiceChanges(true);
                StartDiscovery();
            });
    }

    void PeerServiceDiscovererEcho::ClearUpdatedServices()
    {
        services.erase(std::remove_if(services.begin(), services.end(), [this](uint32_t serviceId)
                           {
                               return serviceId >= SearchRangeBegin() && serviceId <= SearchRangeEnd();
                           }),
            services.end());
    }

    void PeerServiceDiscovererEcho::StartDiscovery()
    {
        ClearUpdatedServices();

        RequestSend([this]
            {
                FindFirstServiceInRange(SearchRangeBegin(), SearchRangeEnd());
            });
    }

    uint32_t PeerServiceDiscovererEcho::SearchRangeBegin() const
    {
        return std::get<0>(searchRange);
    }

    uint32_t PeerServiceDiscovererEcho::SearchRangeEnd() const
    {
        return std::get<1>(searchRange);
    }
}
