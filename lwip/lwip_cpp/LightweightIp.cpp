#include "lwip/lwip_cpp/LightweightIp.hpp"
#include "lwip/init.h"
#ifndef ESP_PLATFORM
#include "lwip/timeouts.h"
#endif

namespace
{
    hal::SynchronousRandomDataGenerator* randomDataGenerator = nullptr;
}

extern "C" uint32_t StaticLwIpRand()
{
    uint32_t result;
    randomDataGenerator->GenerateRandomData(infra::MakeByteRange(result));
    return result;
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

    infra::IntrusiveList<LightweightIp> LightweightIp::instances;
    netif_ext_callback_t LightweightIp::instanceCallback;

    LightweightIp::LightweightIp(AllocatorListenerLwIp& listenerAllocator, infra::BoundedList<ConnectorLwIp>& connectors, AllocatorConnectionLwIp& connectionAllocator,
        hal::SynchronousRandomDataGenerator& randomDataGenerator, infra::CreatorBase<services::Stoppable, void(LightweightIp& lightweightIp)>& connectedCreator)
        : ConnectionFactoryLwIp(listenerAllocator, connectors, connectionAllocator)
        , connectedCreator(connectedCreator)
    {
        ::randomDataGenerator = &randomDataGenerator;
#if NO_SYS
        lwip_init();
        sysCheckTimer.Start(
            std::chrono::milliseconds(50), []()
            {
                sys_check_timeouts();
            },
            infra::triggerImmediately);
#endif

        RegisterInstance();
    }

    LightweightIp::~LightweightIp()
    {
        DeregisterInstance();
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

    void LightweightIp::RegisterInstance()
    {
        if (instances.empty())
            netif_add_ext_callback(&instanceCallback, &InstanceCallback);

        instances.push_back(*this);
    }

    void LightweightIp::DeregisterInstance()
    {
        instances.erase(*this);

        if (instances.empty())
            netif_remove_ext_callback(&instanceCallback);
    }

    void LightweightIp::InstanceCallback(netif* netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t* args)
    {
        for (auto& instance : instances)
            instance.ExtCallback(reason, args);
    }

    void LightweightIp::ExtCallback(netif_nsc_reason_t reason, const netif_ext_callback_args_t* args)
    {
        bool linkUp = (netif_default->flags & NETIF_FLAG_LINK_UP) != 0;

        auto newIpv4Address = GetIPv4Address();

        if (!linkUp)
            newIpv4Address = IPv4Address();

        if ((reason & (LWIP_NSC_IPV4_SETTINGS_CHANGED | LWIP_NSC_LINK_CHANGED)) != 0 && ipv4Address != newIpv4Address)
        {
            ipv4Address = newIpv4Address;

            if (ipv4Address == IPv4Address())
            {
                if (connected != std::nullopt && !stopping)
                {
                    stopping = true;
                    (*connected)->Stop([this]()
                        {
                            OnStopped();
                        });
                }
            }
            else
            {
                if (!stopping)
                {
                    connected.emplace(connectedCreator, *this);
                    starting = false;
                }
                else
                    starting = true;
            }
        }
    }

    void LightweightIp::OnStopped()
    {
        connected = std::nullopt;
        stopping = false;

        if (starting)
        {
            connected.emplace(connectedCreator, *this);
            starting = false;
        }
    }
}
