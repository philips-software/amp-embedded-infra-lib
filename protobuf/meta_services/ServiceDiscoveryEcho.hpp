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

        void FindFirstServiceInRange(uint32_t startServiceId, uint32_t endServiceId) override;
        void NotifyServiceChanges(bool value) override
        {}

        bool AcceptsService(uint32_t id) const override;
        //infra::SharedPtr<services::MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const services::EchoErrorPolicy& errorPolicy) override;

        void RequestSend(ServiceProxy& serviceProxy) override
        {
            service_discovery::ServiceDiscovery::Rpc().RequestSend(serviceProxy);
        }

        void ServiceDone() override
        {
            service_discovery::ServiceDiscovery::Rpc().ServiceDone();
        }

        services::MethodSerializerFactory& SerializerFactory() override
        {
            return service_discovery::ServiceDiscovery::Rpc().SerializerFactory();
        }

    protected:
            bool IsProxyServiceSupported(uint32_t& serviceId) const;
    };
}

#endif
