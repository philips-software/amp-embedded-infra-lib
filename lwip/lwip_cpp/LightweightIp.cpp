#include "lwip/lwip_cpp/LightweightIp.hpp"
#include "lwip/init.h"
#include "lwipopts.h"
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

netif* netifInternal = nullptr;

namespace services
{
    namespace
    {
        IPv4Address Convert(ip_addr_t address)
        {
            return {
                ip4_addr1(&address),
                ip4_addr2(&address),
                ip4_addr3(&address),
                ip4_addr4(&address),
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
        return netifInternal ? Convert(netifInternal->ip_addr) : IPv4Address();
    }

    IPv4InterfaceAddresses LightweightIp::GetIPv4InterfaceAddresses() const
    {
        if (netifInternal)
            return { Convert(netifInternal->ip_addr), Convert(netifInternal->netmask), Convert(netifInternal->gw) };
        else
            return { IPv4Address(), IPv4Address(), IPv4Address() };
    }

    void LightweightIp::RegisterInstance()
    {
        if (instances.empty())
        {
            LOCK_TCPIP_CORE();
            netif_add_ext_callback(&instanceCallback, &InstanceCallback);
            UNLOCK_TCPIP_CORE();
        }

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
        netifInternal = netif;
        for (auto& instance : instances)
            instance.ExtCallback(reason, args);
    }

    void LightweightIp::ExtCallback(netif_nsc_reason_t reason, const netif_ext_callback_args_t* args)
    {
        if (!netifInternal)
        {
            return;
        }

        bool linkUp = (netifInternal->flags & NETIF_FLAG_LINK_UP) != 0;

        auto newIpv4Address = GetIPv4Address();

        if (!linkUp)
            newIpv4Address = IPv4Address();

        if ((reason & (LWIP_NSC_IPV4_SETTINGS_CHANGED | LWIP_NSC_LINK_CHANGED)) != 0 && ipv4Address != newIpv4Address)
        {
            ipv4Address = newIpv4Address;

            if (ipv4Address == IPv4Address())
            {
                if (connected != infra::none && !stopping)
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
                    connected.Emplace(connectedCreator, *this);
                    starting = false;
                }
                else
                    starting = true;
            }
        }
    }

    void LightweightIp::OnStopped()
    {
        connected = infra::none;
        stopping = false;

        if (starting)
        {
            connected.Emplace(connectedCreator, *this);
            starting = false;
        }
    }
}
