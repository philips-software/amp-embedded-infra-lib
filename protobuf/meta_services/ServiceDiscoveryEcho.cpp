#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"

namespace application {
    void ServiceDiscoveryEcho::FindFirstServiceInRange(uint32_t startServiceId, uint32_t endServiceId)
    {
        infra::Optional<uint32_t> service;

        for(auto id = startServiceId; id <= endServiceId; ++id)
        {
            if(IsServiceSupported(id))
                {
                    service = id;
                    break;
                }
        }

        RequestSend([this, service]
            {
                if (service)
                    FirstServiceSupported(*service);
                else
                    NoServiceSupported();
            });

        MethodDone();
    }

    bool ServiceDiscoveryEcho::IsServiceSupported(uint32_t serviceId)
    {
        auto serviceFound = false;
        service_discovery::ServiceDiscovery::Rpc().NotifyObservers([serviceId, &serviceFound](auto& observer)
        {
            if (observer.AcceptsService(serviceId))
                serviceFound = true;
        });

        return serviceFound;
    }
}

