#ifndef PROTOBUF_ECHO_SERVICE_DISCOVERY_HPP
#define PROTOBUF_ECHO_SERVICE_DISCOVERY_HPP

#include "generated/echo/ServiceDiscovery.pb.hpp"
#include <cstdint>

namespace application
{
    class ServiceDiscoveryEcho
        : public service_discovery::ServiceDiscovery
        , public service_discovery::ServiceDiscoveryResponseProxy
    {
    public:
        explicit ServiceDiscoveryEcho(services::Echo& echo)
            : service_discovery::ServiceDiscovery(echo)
            , service_discovery::ServiceDiscoveryResponseProxy(echo)
        {}

        virtual ~ServiceDiscoveryEcho() = default;

        void FindFirstServiceInRange(uint32_t startServiceId, uint32_t endServiceId) override;

    private:
        bool IsServiceSupported(uint32_t serviceId);
    };
}

#endif
