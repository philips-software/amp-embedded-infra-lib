#include <cstdlib>
#include <cstring>
#include "infra/event/EventDispatcher.hpp"
#include "lwip/lwip_cpp/LightweightIpOverEthernet.hpp"
#include "lwip/dhcp.h"
#include "lwip/ethip6.h"
#include "lwip/igmp.h"
#include "netif/etharp.h"

namespace services
{
    LightweightIpOverEthernet::LightweightIpOverEthernet(hal::EthernetMac& ethernet, netif& netInterface)
        : hal::EthernetMacObserver(ethernet)
        , netInterface(netInterface)
    {
        netif_set_link_up(&netInterface);

        for (auto group = netif_igmp_data(&netInterface); group != nullptr; group = group->next)
            SetIgmpMacFilter(&group->group_address, NETIF_ADD_MAC_FILTER);
    }

    LightweightIpOverEthernet::~LightweightIpOverEthernet()
    {
        if (currentReceiveBufferFirst)
            pbuf_free(currentReceiveBufferFirst);

        netif_set_link_down(&netInterface);

        for (auto group = netif_igmp_data(&netInterface); group != nullptr; group = group->next)
            SetIgmpMacFilter(&group->group_address, NETIF_DEL_MAC_FILTER);
    }

    infra::ByteRange LightweightIpOverEthernet::RequestReceiveBuffer()
    {
        pbuf* buffer = pbuf_alloc(PBUF_RAW, LWIP_MEM_ALIGN_SIZE(PBUF_POOL_BUFSIZE), PBUF_POOL);
        if (buffer == nullptr)
        {
            retryAllocationTimer.Start(std::chrono::milliseconds(250), [this]() { Subject().RetryAllocation(); });
            return infra::ByteRange();
        }

        assert(buffer->next == nullptr);

        if (currentReceiveBufferFirst == nullptr)
            currentReceiveBufferFirst = buffer;
        else
            currentReceiveBufferLast->next = buffer;
        currentReceiveBufferLast = buffer;

        return infra::ByteRange(reinterpret_cast<uint8_t*>(buffer->payload), reinterpret_cast<uint8_t*>(buffer->payload) + buffer->len);
    }

    void LightweightIpOverEthernet::ReceivedFrame(uint32_t usedBuffers, uint32_t frameSize)
    {
        assert(currentReceiveBufferFirst != nullptr);

        pbuf* end = currentReceiveBufferFirst;
        end->tot_len = frameSize;
        while (--usedBuffers != 0)
        {
            frameSize -= end->len;
            end = end->next;
        }

        assert(end != nullptr);
        end->len = frameSize;

        pbuf* newBegin = end->next;
        end->next = nullptr;

        err_t err = netInterface.input(currentReceiveBufferFirst, &netInterface);

        if (err != ERR_OK)
            pbuf_free(currentReceiveBufferFirst);

        currentReceiveBufferFirst = newBegin;
        if (currentReceiveBufferFirst == nullptr)
            currentReceiveBufferLast = nullptr;
    }

    void LightweightIpOverEthernet::ReceivedErrorFrame(uint32_t usedBuffers, uint32_t frameSize)
    {
        pbuf* end = currentReceiveBufferFirst;
        while (--usedBuffers != 0)
            end = end->next;

        assert(end != nullptr);
        pbuf* newBegin = end->next;
        end->next = nullptr;

        pbuf_free(currentReceiveBufferFirst);

        currentReceiveBufferFirst = newBegin;
        if (currentReceiveBufferFirst == nullptr)
            currentReceiveBufferLast = nullptr;
    }

    void LightweightIpOverEthernet::SentFrame()
    {
        pbuf_free(currentSendFrames.front());
        currentSendFrames.pop_front();
        sending = false;

        if (!currentSendFrames.empty())
            SendOneFrame();
    }

    err_t LightweightIpOverEthernet::Output(pbuf* p)
    {
        if (!currentSendFrames.full())
        {
            pbuf_ref(p);
            currentSendFrames.push_back(p);
            if (!sending)
                SendOneFrame();

            return ERR_OK;
        }
        else
            return ERR_MEM;
    }

    err_t LightweightIpOverEthernet::SetMldMacFilter(const ip6_addr_t* group, netif_mac_filter_action action)
    {
        hal::MacAddress address = { 0x33, 0x33, static_cast<uint8_t>(IP6_ADDR_BLOCK7(group) >> 8), static_cast<uint8_t>(IP6_ADDR_BLOCK7(group)),
            static_cast<uint8_t>(IP6_ADDR_BLOCK8(group) >> 8), static_cast<uint8_t>(IP6_ADDR_BLOCK8(group)) };

        if (action == NETIF_ADD_MAC_FILTER)
            Subject().AddMacAddressFilter(address);
        else
            Subject().RemoveMacAddressFilter(address);

        return ERR_OK;
    }

    err_t LightweightIpOverEthernet::SetIgmpMacFilter(const ip4_addr_t* group, netif_mac_filter_action action)
    {
        hal::MacAddress address = { 0x01, 0x00, 0x5e, static_cast<uint8_t>(ip4_addr2(group) & 0x7f), ip4_addr3(group), ip4_addr4(group) };

        if (action == NETIF_ADD_MAC_FILTER)
            Subject().AddMacAddressFilter(address);
        else
            Subject().RemoveMacAddressFilter(address);

        return ERR_OK;
    }

    void LightweightIpOverEthernet::SendOneFrame()
    {
        sending = true;
        pbuf* buffer = currentSendFrames.front();

        while (true)
        {
            bool lastOfFrame = buffer->tot_len == buffer->len;

            Subject().SendBuffer(infra::ConstByteRange(static_cast<const uint8_t*>(buffer->payload), static_cast<const uint8_t*>(buffer->payload) + buffer->len)
                , lastOfFrame);

            if (lastOfFrame)
                break;

            buffer = buffer->next;
        }
    }

    LightweightIpOverEthernetFactory::LightweightIpOverEthernetFactory(hal::MacAddress macAddress, const Config& config)
        : macAddress(macAddress)
        , config(config)
    {
        ip_addr_t ipaddr = IPADDR4_INIT(0);
        ip_addr_t netmask = IPADDR4_INIT(0);
        ip_addr_t gw = IPADDR4_INIT(0);

        if (!config.ipConfig.useDhcp)
        {
            IP4_ADDR(ip_2_ip4(&ipaddr), config.ipConfig.staticAddresses.address[0], config.ipConfig.staticAddresses.address[1], config.ipConfig.staticAddresses.address[2], config.ipConfig.staticAddresses.address[3]);
            IP4_ADDR(ip_2_ip4(&netmask), config.ipConfig.staticAddresses.netmask[0], config.ipConfig.staticAddresses.netmask[1], config.ipConfig.staticAddresses.netmask[2], config.ipConfig.staticAddresses.netmask[3]);
            IP4_ADDR(ip_2_ip4(&gw), config.ipConfig.staticAddresses.gateway[0], config.ipConfig.staticAddresses.gateway[1], config.ipConfig.staticAddresses.gateway[2], config.ipConfig.staticAddresses.gateway[3]);
        }

        netif_add(&netInterface, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw), this, &LightweightIpOverEthernetFactory::StaticInit, &ethernet_input);
        netif_set_up(&netInterface);
        netif_set_default(&netInterface);

        netif_set_hostname(&netInterface, config.hostName.Storage().data());
        netif_set_ip6_autoconfig_enabled(&netInterface, 1);

        netif_create_ip6_linklocal_address(&netInterface, 1);
        netif_ip6_addr_set_state(&netInterface, 0, IP6_ADDR_TENTATIVE);

        netif_set_mld_mac_filter(&netInterface, &LightweightIpOverEthernetFactory::StaticSetMldMacFilter);
        netif_set_igmp_mac_filter(&netInterface, &LightweightIpOverEthernetFactory::StaticSetIgmpMacFilter);

        if (config.ipConfig.useDhcp)
            dhcp_start(&netInterface);
    }

    LightweightIpOverEthernetFactory::~LightweightIpOverEthernetFactory()
    {
        if (config.ipConfig.useDhcp)
            dhcp_stop(&netInterface);
        netif_remove(&netInterface);
    }

    void LightweightIpOverEthernetFactory::Create(hal::EthernetMac& ethernet)
    {
        ethernetStack.Emplace(ethernet, netInterface);
    }

    void LightweightIpOverEthernetFactory::Destroy()
    {
        ethernetStack = infra::none;
    }

    hal::MacAddress LightweightIpOverEthernetFactory::MacAddress() const
    {
        return macAddress;
    }

    err_t LightweightIpOverEthernetFactory::Init()
    {
        netInterface.name[0] = config.ifName[0];
        netInterface.name[1] = config.ifName[1];

        netInterface.output = etharp_output;
        netInterface.output_ip6 = ethip6_output;
        netInterface.linkoutput = &LightweightIpOverEthernetFactory::StaticOutput;

        netInterface.hwaddr_len = static_cast<uint8_t>(macAddress.size());
        netInterface.hwaddr[0] = macAddress[0];
        netInterface.hwaddr[1] = macAddress[1];
        netInterface.hwaddr[2] = macAddress[2];
        netInterface.hwaddr[3] = macAddress[3];
        netInterface.hwaddr[4] = macAddress[4];
        netInterface.hwaddr[5] = macAddress[5];

        netInterface.mtu = 1500;
        netInterface.flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

        return ERR_OK;
    }

    err_t LightweightIpOverEthernetFactory::Output(pbuf* p)
    {
        if (ethernetStack)
            return ethernetStack->Output(p);
        else
            return ERR_IF;
    }

    err_t LightweightIpOverEthernetFactory::SetMldMacFilter(netif* netif, const ip6_addr_t* group, netif_mac_filter_action action)
    {
        if (ethernetStack)
            return ethernetStack->SetMldMacFilter(group, action);
        else
            return ERR_IF;
    }

    err_t LightweightIpOverEthernetFactory::SetIgmpMacFilter(netif* netif, const ip4_addr_t* group, netif_mac_filter_action action)
    {
        if (ethernetStack)
            return ethernetStack->SetIgmpMacFilter(group, action);
        else
            return ERR_IF;
    }

    err_t LightweightIpOverEthernetFactory::StaticOutput(netif* netif, pbuf* buffer)
    {
        return static_cast<LightweightIpOverEthernetFactory*>(netif->state)->Output(buffer);
    }

    err_t LightweightIpOverEthernetFactory::StaticInit(netif* netif)
    {
        return static_cast<LightweightIpOverEthernetFactory*>(netif->state)->Init();
    }

    err_t LightweightIpOverEthernetFactory::StaticSetMldMacFilter(netif* netif, const ip6_addr_t* group, netif_mac_filter_action action)
    {
        return static_cast<LightweightIpOverEthernetFactory*>(netif->state)->SetMldMacFilter(netif, group, action);
    }

    err_t LightweightIpOverEthernetFactory::StaticSetIgmpMacFilter(netif* netif, const ip4_addr_t* group, netif_mac_filter_action action)
    {
        return static_cast<LightweightIpOverEthernetFactory*>(netif->state)->SetIgmpMacFilter(netif, group, action);
    }
}
