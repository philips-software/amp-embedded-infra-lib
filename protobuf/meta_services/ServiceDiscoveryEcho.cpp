#include "protobuf/meta_services/ServiceDiscoveryEcho.hpp"
#include "echo/ServiceDiscovery.pb.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "protobuf/echo/Echo.hpp"
#include "services/tracer/GlobalTracer.hpp"
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

            serviceSupportedClaimer.Claim([this, serviceId]
                {
                    service_discovery::ServiceDiscoveryResponseProxy::RequestSend([this, serviceId]
                        {
                            FirstServiceSupported(serviceId);
                            serviceSupportedClaimer.Release();
                        });
                });
        }
        else
        {
            serviceSupportedClaimer.Claim([this]
                {
                    service_discovery::ServiceDiscoveryResponseProxy::RequestSend([this]
                        {
                            NoServiceSupported();
                            serviceSupportedClaimer.Release();
                        });
                });
        }

        MethodDone();
    }

    uint32_t ServiceDiscoveryEcho::GetServiceId(infra::Observer<Service, Echo>& observer) const
    {
        return services::ServiceIdAccess::GetId(*static_cast<services::ServiceId*>(static_cast<Service*>(&observer)));
    }

    infra::Optional<uint32_t> ServiceDiscoveryEcho::FirstSupportedServiceId(uint32_t startServiceId, uint32_t endServiceId)
    {
        struct FirstSupportedServiceQuery
        {
            void UpdateServiceId(uint32_t id)
            {
                if (id >= startServiceId && id <= endServiceId)
                {
                    if (serviceId)
                        *serviceId = std::min(*serviceId, id);
                    else
                        serviceId = infra::MakeOptional(id);
                }
            }

            const uint32_t startServiceId;
            const uint32_t endServiceId;
            infra::Optional<uint32_t> serviceId;
        };

        FirstSupportedServiceQuery query{ startServiceId, endServiceId };

        services::Echo::NotifyObservers([&query, this](auto& observer)
            {
                query.UpdateServiceId(GetServiceId(observer));
            });

        query.UpdateServiceId(GetServiceId(*this));

        return query.serviceId;
    }

    void ServiceDiscoveryEcho::NotifyServiceChanges(bool value)
    {
        notifyServiceChanges = value;

        if (!notifyServiceChanges)
            changedServices = infra::none;

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

    void ServiceDiscoveryEcho::CancelRequestSend(ServiceProxy& serviceProxy)
    {
        UpstreamRpc().CancelRequestSend(serviceProxy);
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
        auto id = GetServiceId(*observer);
        ServicesChangeNotification(id);

        services::Echo::RegisterObserver(observer);
    }

    void ServiceDiscoveryEcho::UnregisterObserver(infra::Observer<Service, Echo>* observer)
    {
        auto id = services::ServiceIdAccess::GetId(*static_cast<services::ServiceId*>(static_cast<Service*>(observer)));
        ServicesChangeNotification(id);

        services::Echo::UnregisterObserver(observer);
    }

    void ServiceDiscoveryEcho::UpdateChangedServices(uint32_t& serviceId)
    {
        if (!changedServices)
            changedServices = std::make_pair(serviceId, serviceId);
        else
            changedServices = std::make_pair(std::min(changedServices->first, serviceId), std::max(changedServices->second, serviceId));
    }

    void ServiceDiscoveryEcho::ServicesChangeNotification(uint32_t serviceId)
    {
        if (notifyServiceChanges)
        {
            UpdateChangedServices(serviceId);

            if (!serviceChangeNotificationTimer.Armed())
                serviceChangeNotificationTimer.Start(std::chrono::milliseconds(500), [this]
                    {
                        if (!servicesChangedClaimer.IsQueued())
                            servicesChangedClaimer.Claim([this]
                                {
                                    service_discovery::ServiceDiscoveryResponseProxy::RequestSend([this]
                                        {
                                            ServicesChanged(changedServices->first, changedServices->second);
                                            changedServices = infra::none;
                                            servicesChangedClaimer.Release();
                                        });
                                });
                    });
        }
    }
}
