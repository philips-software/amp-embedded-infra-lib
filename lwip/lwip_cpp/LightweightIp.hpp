#ifndef LWIP_LIGHTWEIGHT_IP_HPP
#define LWIP_LIGHTWEIGHT_IP_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/util/ProxyCreator.hpp"
#include "lwip/lwip_cpp/ConnectionLwIp.hpp"
#include "lwip/lwip_cpp/DatagramLwIp.hpp"
#include "lwip/lwip_cpp/MulticastLwIp.hpp"
#include "services/network/ConnectionStatus.hpp"
#include "services/util/Stoppable.hpp"

namespace services
{
    class LightweightIp
        : public ConnectionFactoryLwIp
        , public DatagramFactoryLwIp
        , public MulticastLwIp
        , public IPv4Info
        , public ConnectionStatus
        , public infra::IntrusiveList<LightweightIp>::NodeType
    {
    public:
        template<std::size_t MaxListeners, std::size_t MaxConnectors, std::size_t MaxConnections>
        using WithFixedAllocator = infra::WithStorage<infra::WithStorage<infra::WithStorage<LightweightIp,
                                                                             AllocatorListenerLwIp::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxListeners>>,
                                                          infra::BoundedList<ConnectorLwIp>::WithMaxSize<MaxConnectors>>,
            AllocatorConnectionLwIp::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<MaxConnections>>;

        LightweightIp(AllocatorListenerLwIp& listenerAllocator, infra::BoundedList<ConnectorLwIp>& connectors, AllocatorConnectionLwIp& connectionAllocator,
            hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::CreatorBase<services::Stoppable, void(LightweightIp& lightweightIp)>& connectedCreator);
        ~LightweightIp() override;

        // Implementation of ConnectionStatus
        bool PendingSend() const override;

        // Implementation of IPv4Info
        IPv4Address GetIPv4Address() const override;
        IPv4InterfaceAddresses GetIPv4InterfaceAddresses() const override;

    private:
        void RegisterInstance();
        void DeregisterInstance();
        static void InstanceCallback(netif* netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t* args);

        void ExtCallback(netif_nsc_reason_t reason, const netif_ext_callback_args_t* args);
        void OnStopped();

    private:
        infra::TimerRepeating sysCheckTimer;

        services::IPv4Address ipv4Address;

        infra::CreatorBase<services::Stoppable, void(LightweightIp& lightweightIp)>& connectedCreator;
        std::optional<infra::ProxyCreator<services::Stoppable, void(LightweightIp& lightweightIp)>> connected;
        bool stopping = false;
        bool starting = false;

        static infra::IntrusiveList<LightweightIp> instances;
        static netif_ext_callback_t instanceCallback;
    };
}

#endif
