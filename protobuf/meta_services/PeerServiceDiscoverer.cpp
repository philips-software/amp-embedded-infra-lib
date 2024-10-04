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
        Initialize();
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
        Initialize();

        MethodDone();
    }

    void PeerServiceDiscovererEcho::Initialize()
    {
        services.clear();

        RequestSend([this]
            {
                NotifyServiceChanges(true);
                StartDiscovery();
            });
    }

    void PeerServiceDiscovererEcho::StartDiscovery()
    {
        RequestSend([this]
            {
                FindFirstServiceInRange(0, std::numeric_limits<uint32_t>::max());
            });
    }
}
