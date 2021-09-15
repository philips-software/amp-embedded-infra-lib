#include "lwip/lwip_cpp/LightweightIp.hpp"
#include "lwip/init.h"
#ifndef ESP_PLATFORM
#include "lwip/timeouts.h"
#endif

extern "C" uint32_t StaticLwIpRand()
{
    return services::LightweightIp::Instance().Rand();
}

namespace services
{
    namespace
    {
        IPv4Address Convert(ip_addr_t address)
        {
            return {
                ip4_addr1(&address.u_addr.ip4),
                ip4_addr2(&address.u_addr.ip4),
                ip4_addr3(&address.u_addr.ip4),
                ip4_addr4(&address.u_addr.ip4),
            };
        }
    }

    LightweightIp::LightweightIp(AllocatorListenerLwIp& listenerAllocator, infra::BoundedList<ConnectorLwIp>& connectors, AllocatorConnectionLwIp& connectionAllocator, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : ConnectionFactoryLwIp(listenerAllocator, connectors, connectionAllocator)
        , randomDataGenerator(randomDataGenerator)
    {
#if NO_SYS
        lwip_init();
        sysCheckTimer.Start(
            std::chrono::milliseconds(50), [this]() { sys_check_timeouts(); }, infra::triggerImmediately);
#endif
    }

    uint32_t LightweightIp::Rand()
    {
        uint32_t result;
        randomDataGenerator.GenerateRandomData(infra::MakeByteRange(result));
        return result;
    }

    bool LightweightIp::PendingSend() const
    {
        return ConnectionFactoryLwIp::PendingSend();
    }

    IPv4Address LightweightIp::GetIPv4Address() const
    {
        return Convert(netif_default->ip_addr);
    }

    IPv4InterfaceAddresses LightweightIp::GetIPv4InterfaceAddresses() const
    {
        return { Convert(netif_default->ip_addr), Convert(netif_default->netmask), Convert(netif_default->gw) };
    }
}
