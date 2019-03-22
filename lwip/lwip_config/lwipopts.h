#ifndef LWIP_LWIPOPTS_H
#define LWIP_LWIPOPTS_H

#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NO_SYS 1
#define LWIP_IPV6 1
#define SYS_LIGHTWEIGHT_PROT 0
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0
#define LWIP_DHCP 1
#define LWIP_DHCP_CHECK_LINK_UP 1
#define LWIP_AUTOIP 1
#define LWIP_DHCP_AUTOIP_COOP 1
#define LWIP_DNS 1
#define LWIP_RAW 0
#define LWIP_IGMP 1
uint32_t StaticLwIpRand();
#define LWIP_RAND() StaticLwIpRand()
#define MEM_ALIGNMENT 4
#define LWIP_STATS 0

#define LWIP_DNS_SECURE 0

#define MEM_SIZE 3200

#define CHECKSUM_GEN_IP 0
#define CHECKSUM_GEN_UDP 0
#define CHECKSUM_GEN_TCP 0
#define CHECKSUM_GEN_ICMP 0
#define CHECKSUM_CHECK_IP 0
#define CHECKSUM_CHECK_UDP 0
#define CHECKSUM_CHECK_TCP 0

#define LWIP_NETIF_HOSTNAME 1

#ifdef __cplusplus
}
#endif

#endif
