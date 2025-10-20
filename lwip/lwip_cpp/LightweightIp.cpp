#include "lwip/lwip_cpp/LightweightIp.hpp"
#include "lwip/init.h"
#include "services/network/Address.hpp"
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
        IPv4Address Convert(const ip4_addr_t& address)
        {
            return {
                ip4_addr1(&address),
                ip4_addr2(&address),
                ip4_addr3(&address),
                ip4_addr4(&address),
            };
        }

        IPv6Address Convert(const ip6_addr_t& address)
        {
            return {
                IP6_ADDR_BLOCK1(&address),
                IP6_ADDR_BLOCK2(&address),
                IP6_ADDR_BLOCK3(&address),
                IP6_ADDR_BLOCK4(&address),
                IP6_ADDR_BLOCK5(&address),
                IP6_ADDR_BLOCK6(&address),
                IP6_ADDR_BLOCK7(&address),
                IP6_ADDR_BLOCK8(&address)
            };
        }

        IPAddress Convert(const ip_addr_t& address)
        {
            switch (address.type)
            {
                case IPADDR_TYPE_V4:
                    return Convert(address.u_addr.ip4);
                case IPADDR_TYPE_V6:
                    return Convert(address.u_addr.ip6);
                default:
                    return {};
            }
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
        return Convert(netif_default->ip_addr.u_addr.ip4);
    }

    IPv4InterfaceAddresses LightweightIp::GetIPv4InterfaceAddresses() const
    {
        return { Convert(netif_default->ip_addr.u_addr.ip4), Convert(netif_default->netmask.u_addr.ip4), Convert(netif_default->gw.u_addr.ip4) };
    }

    IPv6Address LightweightIp::LinkLocalAddress() const
    {
        for (const auto& address : netif_default->ip6_addr)
            if (ip6_addr_islinklocal(&address.u_addr.ip6))
                return Convert(address.u_addr.ip6);
        return {};
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
        if (netif_default == nullptr)
            return;

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
        connected.reset();
        stopping = false;

        if (starting)
        {
            connected.emplace(connectedCreator, *this);
            starting = false;
        }
    }
}
