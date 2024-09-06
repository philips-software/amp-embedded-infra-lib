#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/Optional.hpp"
#include "protobuf/echo/Echo.hpp"
#include <cstdint>
#include <tuple>
#include <utility>

namespace application
{
    void ServiceDiscoveryEcho::FindFirstServiceInRange(uint32_t startServiceId, uint32_t endServiceId)
    {
        if (auto service = FirstSupportedServiceId(startServiceId, endServiceId); service)
        {
            auto serviceId = *service;
            service_discovery::ServiceDiscoveryResponseProxy::RequestSend([this, serviceId]
                {
                    FirstServiceSupported(serviceId);
                });
        }
        else
            service_discovery::ServiceDiscoveryResponseProxy::RequestSend([this]
                {
                    NoServiceSupported();
                });

        MethodDone();
    }

    infra::Optional<uint32_t> ServiceDiscoveryEcho::FirstSupportedServiceId(uint32_t startServiceId, uint32_t endServiceId)
    {
        for (auto id = startServiceId; id <= endServiceId; ++id)
            if (AcceptsService(id))
                return infra::MakeOptional(id);

        return infra::none;
    }

    void ServiceDiscoveryEcho::NotifyServiceChanges(bool value)
    {
        notifyServiceChanges = value;

        MethodDone();
    }

    bool ServiceDiscoveryEcho::AcceptsService(uint32_t serviceId) const
    {
        return service_discovery::ServiceDiscovery::AcceptsService(serviceId) || IsProxyServiceSupported(serviceId);
    }

    infra::SharedPtr<services::MethodDeserializer> ServiceDiscoveryEcho::
        StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const services::EchoErrorPolicy& errorPolicy)
    {
        if (service_discovery::ServiceDiscovery::AcceptsService(serviceId))
            return service_discovery::ServiceDiscovery::StartMethod(serviceId, methodId, size, errorPolicy);
        else
            return StartProxyServiceMethod(serviceId, methodId, size, errorPolicy);
    }

    infra::SharedPtr<services::MethodDeserializer> ServiceDiscoveryEcho::
        StartProxyServiceMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const services::EchoErrorPolicy& errorPolicy)
    {
        infra::SharedPtr<services::MethodDeserializer> methodSerializer;

        auto startMethodArgs = std::tie(serviceId, methodId, size, errorPolicy);
        NotifyObservers([&methodSerializer, &startMethodArgs](auto& obs)
            {
                if (obs.AcceptsService(std::get<0>(startMethodArgs)))
                {
                    methodSerializer = obs.StartMethod(std::get<0>(startMethodArgs), std::get<1>(startMethodArgs),
                        std::get<2>(startMethodArgs), std::get<3>(startMethodArgs));
                    return true;
                }

                return false;
            });

        return methodSerializer;
    }

    bool ServiceDiscoveryEcho::IsProxyServiceSupported(uint32_t serviceId) const
    {
        return services::Echo::NotifyObservers([serviceId](auto& observer)
            {
                return observer.AcceptsService(serviceId);
            });
    }

    void ServiceDiscoveryEcho::RequestSend(ServiceProxy& serviceProxy)
    {
        UpstreamRpc().RequestSend(serviceProxy);
    }

    void ServiceDiscoveryEcho::ServiceDone()
    {
        UpstreamRpc().ServiceDone();
    }

    services::MethodSerializerFactory& ServiceDiscoveryEcho::SerializerFactory()
    {
        return UpstreamRpc().SerializerFactory();
    }

    services::Echo& ServiceDiscoveryEcho::UpstreamRpc()
    {
        return service_discovery::ServiceDiscovery::Rpc();
    }

    void ServiceDiscoveryEcho::RegisterObserver(infra::Observer<Service, Echo>* observer)
    {
        auto id = services::ServiceIdAccess::GetId(*static_cast<services::ServiceId*>(static_cast<Service*>(observer)));
        ServicesChangeNotification(id);

        services::Echo::RegisterObserver(observer);
    }

    void ServiceDiscoveryEcho::UnregisterObserver(infra::Observer<Service, Echo>* observer)
    {
        auto id = services::ServiceIdAccess::GetId(*static_cast<services::ServiceId*>(static_cast<Service*>(observer)));
        ServicesChangeNotification(id);

        services::Echo::UnregisterObserver(observer);
    }

    void ServiceDiscoveryEcho::ServicesChangeNotification(uint32_t serviceId)
    {
        if (!changedServices)
            changedServices = std::make_pair(serviceId, serviceId);
        else
            changedServices = std::make_pair(std::min(changedServices->first, serviceId), std::max(changedServices->second, serviceId));

        if (notifyServiceChanges)
        {
            service_discovery::ServiceDiscoveryResponseProxy::RequestSend([this]
                {
                    ServicesChanged(changedServices->first, changedServices->second);
                    changedServices = infra::none;
                });
        }
    }
}
