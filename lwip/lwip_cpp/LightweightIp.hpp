#ifndef LWIP_LIGHTWEIGHT_IP_HPP
#define LWIP_LIGHTWEIGHT_IP_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/InterfaceConnector.hpp"
#include "lwip/lwip_cpp/ConnectionLwIp.hpp"
#include "lwip/lwip_cpp/DatagramLwIp.hpp"
#include "lwip/lwip_cpp/MulticastLwIp.hpp"

namespace services
{
    class LightweightIp
        : public ConnectionFactoryLwIp
        , public DatagramFactoryLwIp
        , public MulticastLwIp
        , public IPv4Info
        , public infra::InterfaceConnector<LightweightIp>
    {
    public:
        template<std::size_t MaxListeners, std::size_t MaxConnectors, std::size_t MaxConnections>
            using WithFixedAllocator = infra::WithStorage<infra::WithStorage<infra::WithStorage<LightweightIp,
                AllocatorListenerLwIp::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxListeners>>,
                infra::BoundedList<ConnectorLwIp>::WithMaxSize<MaxConnectors>>,
                AllocatorConnectionLwIp::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>;

        LightweightIp(AllocatorListenerLwIp& listenerAllocator, infra::BoundedList<ConnectorLwIp>& connectors, AllocatorConnectionLwIp& connectionAllocator, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        uint32_t Rand();

        // Implementation of IPv4Info
        virtual IPv4Address GetIPv4Address() const override;
        virtual IPv4InterfaceAddresses GetIPv4InterfaceAddresses() const override;

    private:
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::TimerRepeating sysCheckTimer;
    };
}

#endif
