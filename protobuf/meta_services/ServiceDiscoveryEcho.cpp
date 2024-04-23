#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "echo/ServiceDiscovery.pb.hpp"

namespace application
{
    void ServiceDiscoveryEcho::FindFirstServiceInRange(uint32_t startServiceId, uint32_t endServiceId)
    {
        infra::Optional<uint32_t> service;

        for (auto id = startServiceId; id <= endServiceId; ++id)
        {
            if (AcceptsService(id))
            {
                service = id;
                break;
            }
        }

        service_discovery::ServiceDiscoveryResponseProxy::RequestSend([this, service]
            {
                if (service)
                    FirstServiceSupported(*service);
                else
                    NoServiceSupported();
            });

        MethodDone();
    }

    void ServiceDiscoveryEcho::NotifyServiceChanges(bool value)
    {}

    bool ServiceDiscoveryEcho::AcceptsService(uint32_t serviceId) const
    {
        return service_discovery::ServiceDiscovery::AcceptsService(serviceId) 
                || IsProxyServiceSupported(serviceId);
    }

    bool ServiceDiscoveryEcho::IsProxyServiceSupported(uint32_t& serviceId) const
    {
        auto serviceFound = false;
        services::Echo::NotifyObservers([serviceId, &serviceFound](auto& observer)
            {
                if (observer.AcceptsService(serviceId))
                    serviceFound = true;
            });

        return serviceFound;
    }

    void ServiceDiscoveryEcho::RequestSend(ServiceProxy& serviceProxy)
    {
        service_discovery::ServiceDiscovery::Rpc().RequestSend(serviceProxy);
    }

    void ServiceDiscoveryEcho::ServiceDone()
    {
        service_discovery::ServiceDiscovery::Rpc().ServiceDone();
    }

    services::MethodSerializerFactory& ServiceDiscoveryEcho::SerializerFactory()
    {
        return service_discovery::ServiceDiscovery::Rpc().SerializerFactory();
    }
}
