#ifndef PROTOBUF_ECHO_SERVICE_DISCOVERY_HPP
#define PROTOBUF_ECHO_SERVICE_DISCOVERY_HPP

#include "generated/echo/ServiceDiscovery.pb.hpp"
#include "protobuf/echo/Echo.hpp"
#include <cstdint>

namespace application
{
    class ServiceDiscoveryEcho
        : public service_discovery::ServiceDiscovery
        , public service_discovery::ServiceDiscoveryResponseProxy
        , public services::Echo
    {
    public:
        explicit ServiceDiscoveryEcho(services::Echo& echo)
            : service_discovery::ServiceDiscovery(echo)
            , service_discovery::ServiceDiscoveryResponseProxy(echo)
        {}

        virtual ~ServiceDiscoveryEcho() = default;

        //Implementation of services::ServiceDiscovery
        void FindFirstServiceInRange(uint32_t startServiceId, uint32_t endServiceId) override;
        void NotifyServiceChanges(bool value) override;

        //Implementation of services::Service
        bool AcceptsService(uint32_t id) const override;
        infra::SharedPtr<services::MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const services::EchoErrorPolicy& errorPolicy) override;

        //Implementation of services::Echo
        void RequestSend(ServiceProxy& serviceProxy) override;
        void ServiceDone() override;
        services::MethodSerializerFactory& SerializerFactory() override;

        void RegisterObserver(infra::Observer<Service, Echo>* observer) override;
        void UnregisterObserver(infra::Observer<Service, Echo>* observer) override;

    private:
        infra::Optional<uint32_t> FirstSupportedServiceId(uint32_t startServiceId, uint32_t endServiceId);
        bool IsProxyServiceSupported(uint32_t serviceId) const;
        infra::SharedPtr<services::MethodDeserializer> StartProxyServiceMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const services::EchoErrorPolicy& errorPolicy);
        services::Echo& UpstreamRpc();
        void ServicesChangeNotification();

    private:
        bool notifyServiceChanges = false;
    };
}

#endif
