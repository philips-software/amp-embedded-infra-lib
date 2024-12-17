/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_LWIPOPTS_H
#define LWIP_LWIPOPTS_H

#ifdef LWIP_OPTTEST_FILE
#include "lwipopts_test.h"
#else /* LWIP_OPTTEST_FILE */

#define LWIP_IPV4 1
#define LWIP_IPV6 1

#define NO_SYS 0
#define LWIP_SOCKET (NO_SYS == 0)
#define LWIP_NETCONN (NO_SYS == 0)
#define LWIP_NETIF_API (NO_SYS == 0)

#define LWIP_IGMP LWIP_IPV4
#define LWIP_ICMP LWIP_IPV4

#define LWIP_SNMP LWIP_UDP
#define MIB2_STATS LWIP_SNMP
#ifdef LWIP_HAVE_MBEDTLS
#define LWIP_SNMP_V3 (LWIP_SNMP)
#endif

#define LWIP_DNS LWIP_UDP
#define LWIP_MDNS_RESPONDER LWIP_UDP

#define LWIP_NUM_NETIF_CLIENT_DATA (LWIP_MDNS_RESPONDER)

#define LWIP_HAVE_LOOPIF 1
#define LWIP_NETIF_LOOPBACK 1
#define LWIP_LOOPBACK_MAX_PBUFS 10

#define TCP_LISTEN_BACKLOG 1

#define LWIP_COMPAT_SOCKETS 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_RCVBUF 1

#define LWIP_TCPIP_CORE_LOCKING 1

#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1

#define LWIP_TCP_PCB_NUM_EXT_ARGS 1
#define LWIP_NETIF_HOSTNAME 1

// #define LWIP_DEBUG			0

#ifdef LWIP_DEBUG

#define MEM_STATS 1
#define LWIP_DBG_MIN_LEVEL 0
#define PPP_DEBUG LWIP_DBG_OFF
#define MEM_DEBUG LWIP_DBG_OFF
#define MEMP_DEBUG LWIP_DBG_OFF
#define PBUF_DEBUG LWIP_DBG_OFF
#define API_LIB_DEBUG LWIP_DBG_OFF
#define API_MSG_DEBUG LWIP_DBG_OFF
#define TCPIP_DEBUG LWIP_DBG_OFF
#define NETIF_DEBUG LWIP_DBG_OFF
#define SOCKETS_DEBUG LWIP_DBG_OFF
#define DNS_DEBUG LWIP_DBG_OFF
#define AUTOIP_DEBUG LWIP_DBG_OFF
#define DHCP_DEBUG LWIP_DBG_OFF
#define IP_DEBUG LWIP_DBG_OFF
#define IP_REASS_DEBUG LWIP_DBG_OFF
#define ICMP_DEBUG LWIP_DBG_OFF
#define IGMP_DEBUG LWIP_DBG_OFF
#define UDP_DEBUG LWIP_DBG_OFF
#define TCP_DEBUG LWIP_DBG_OFF
#define TCP_INPUT_DEBUG LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG LWIP_DBG_OFF
#define TCP_RTO_DEBUG LWIP_DBG_OFF
#define TCP_CWND_DEBUG LWIP_DBG_OFF
#define TCP_WND_DEBUG LWIP_DBG_OFF
#define TCP_FR_DEBUG LWIP_DBG_OFF
#define TCP_QLEN_DEBUG LWIP_DBG_OFF
#define TCP_RST_DEBUG LWIP_DBG_OFF
#endif

#define LWIP_DBG_TYPES_ON (LWIP_DBG_ON | LWIP_DBG_TRACE | LWIP_DBG_STATE | LWIP_DBG_FRESH | LWIP_DBG_HALT)

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
/* MSVC port: intel processors don't need 4-byte alignment,
   but are faster that way! */
#define MEM_ALIGNMENT 32U
#define MEMP_MEM_MALLOC 1
#define MEM_LIBC_MALLOC 0
// #define MEM_USE_POOLS			0
// #define MEMP_USE_CUSTOM_POOLS	1

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE 32768

/* Debug checks - will impact throughput if enabled */
#define MEM_OVERFLOW_CHECK (0)
#define MEM_SANITY_CHECK (0)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF 10
/* MEMP_NUM_RAW_PCB: the number of UDP protocol control blocks. One
   per active RAW "connection". */
#define MEMP_NUM_RAW_PCB 3
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB 2
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB 3
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 8
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG 128
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT 17

/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF 2
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN 10
/* MEMP_NUM_TCPIP_MSG_*: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#define MEMP_NUM_TCPIP_MSG_API 128
#define MEMP_NUM_TCPIP_MSG_INPKT 128

/* Debug checks - will impact throughput if enabled */
#define MEMP_OVERFLOW_CHECK (0)
#define MEMP_SANITY_CHECK (0)

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE 96

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE 1536

/** SYS_LIGHTWEIGHT_PROT
 * define SYS_LIGHTWEIGHT_PROT in lwipopts.h if you want inter-task protection
 * for certain critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT (NO_SYS == 0)

/* ---------- TCP options ---------- */
#define LWIP_TCP 1
#define TCP_TTL 255

#define LWIP_ALTCP (LWIP_TCP)
#ifdef LWIP_HAVE_MBEDTLS
#define LWIP_ALTCP_TLS (LWIP_TCP)
#define LWIP_ALTCP_TLS_MBEDTLS (LWIP_TCP)
#endif

/* Length of TCP queue equal to number of listening connections +
   number of segments to receive*/
#define TCPIP_MBOX_SIZE (MEMP_NUM_TCP_PCB_LISTEN + MEMP_NUM_TCP_SEG)

/* Number of TCP packets queued for receiving by each TCP connection*/
#define DEFAULT_TCP_RECVMBOX_SIZE (MEMP_NUM_TCP_SEG)

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ 1

/* TCP Maximum segment size. */
#define TCP_MSS 1460

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF TCP_MSS * 8

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN (10 * TCP_SND_BUF / TCP_MSS)

/* TCP writable space (bytes). This must be less than or equal
   to TCP_SND_BUF. It is the amount of space which must be
   available in the tcp snd_buf for select to return writable */
#define TCP_SNDLOWAT (TCP_SND_BUF / 2)

/* TCP receive window. */
#define TCP_WND (20 * 1024)

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX 12

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX 4

#define DEFAULT_THREAD_STACKSIZE (8 * 1024)
#define TCPIP_THREAD_STACKSIZE (16 * 1024) // 16 k

// #define LWIP_FREERTOS_THREAD_STACKSIZE_IS_STACKWORDS (1)

/* ---------- ARP options ---------- */
#define LWIP_ARP 1
#define ARP_TABLE_SIZE 10
#define ARP_QUEUEING 1

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD 1

/* IP reassembly and segmentation.These are orthogonal even
 * if they both deal with IP fragments */
#define IP_REASSEMBLY 1
#define IP_REASS_MAX_PBUFS (10 * ((1500 + PBUF_POOL_BUFSIZE - 1) / PBUF_POOL_BUFSIZE))
#define MEMP_NUM_REASSDATA IP_REASS_MAX_PBUFS
#define IP_FRAG 1
#define IPV6_FRAG_COPYHEADER 1

/* ---------- ICMP options ---------- */
#define ICMP_TTL 255

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. */
#define LWIP_DHCP LWIP_UDP

/* 1 if you want to do an ARP check on the offered address
   (recommended). */
#define DHCP_DOES_ARP_CHECK (LWIP_DHCP)

/* ---------- AUTOIP options ------- */
#define LWIP_AUTOIP (LWIP_DHCP)
#define LWIP_DHCP_AUTOIP_COOP (LWIP_DHCP && LWIP_AUTOIP)

/* ---------- UDP options ---------- */
#define LWIP_UDP 1
#define LWIP_UDPLITE LWIP_UDP
#define UDP_TTL 255

#define DEFAULT_UDP_RECVMBOX_SIZE (MEMP_NUM_TCP_SEG)

/* ---------- RAW options ---------- */
#define LWIP_RAW 1

#define DEFAULT_RAW_RECVMBOX_SIZE (MEMP_NUM_TCP_SEG)

/* ---------- Statistics options ---------- */

#define LWIP_STATS 1
#define LWIP_STATS_DISPLAY 1

#if LWIP_STATS
#define LINK_STATS 1
#define IP_STATS 1
#define ICMP_STATS 1
#define IGMP_STATS 1
#define IPFRAG_STATS 1
#define UDP_STATS 0
#define TCP_STATS 0
#define MEM_STATS 1
#define MEMP_STATS 1
#define PBUF_STATS 1
#define SYS_STATS 1
#endif /* LWIP_STATS */

/* ---------- NETBIOS options ---------- */
#define LWIP_NETBIOS_RESPOND_NAME_QUERY 1

/* ---------- PPP options ---------- */

#define PPP_SUPPORT 0 /* Set > 0 for PPP */

#if PPP_SUPPORT

#define NUM_PPP 1 /* Max PPP sessions. */

/* Select modules to enable.  Ideally these would be set in the makefile but
 * we're limited by the command line length so you need to modify the settings
 * in this file.
 */
#define PPPOE_SUPPORT 1
#define PPPOS_SUPPORT 1

#define PAP_SUPPORT 1    /* Set > 0 for PAP. */
#define CHAP_SUPPORT 1   /* Set > 0 for CHAP. */
#define MSCHAP_SUPPORT 0 /* Set > 0 for MSCHAP */
#define CBCP_SUPPORT 0   /* Set > 0 for CBCP (NOT FUNCTIONAL!) */
#define CCP_SUPPORT 0    /* Set > 0 for CCP */
#define VJ_SUPPORT 1     /* Set > 0 for VJ header compression. */
#define MD5_SUPPORT 1    /* Set > 0 for MD5 (see also CHAP) */

#endif /* PPP_SUPPORT */

#endif /* LWIP_OPTTEST_FILE */

/* The following defines must be done even in OPTTEST mode: */

#if !defined(NO_SYS) || !NO_SYS /* default is 0 */
void sys_check_core_locking(void);
#define LWIP_ASSERT_CORE_LOCKED() sys_check_core_locking()
void sys_mark_tcpip_thread(void);
#define LWIP_MARK_TCPIP_THREAD() sys_mark_tcpip_thread()

#if !defined(LWIP_TCPIP_CORE_LOCKING) || LWIP_TCPIP_CORE_LOCKING /* default is 1 */
void sys_lock_tcpip_core(void);
#define LOCK_TCPIP_CORE() sys_lock_tcpip_core()
void sys_unlock_tcpip_core(void);
#define UNLOCK_TCPIP_CORE() sys_unlock_tcpip_core()
#endif
#endif

/*-------------------------Misc Options-------------------------*/
/* Enables ARP*/
#define LWIP_ARP 1

/* Checksum on copy from app buffers to pbufs, boosts performance */
#define LWIP_CHECKSUM_ON_COPY 1

/* Enables a routine to be called when netif is deleted. Used to close CPSW*/
#define LWIP_NETIF_REMOVE_CALLBACK 1

/* TCP thread priority level*/
#define TCPIP_THREAD_PRIO 6

/* Thread priority of any other thread created using the stack functions */
#define DEFAULT_THREAD_PRIO 1

/* Prevents pbuf chain from getting created thus disabling scatter-gather*/
#define LWIP_NETIF_TX_SINGLE_PBUF 1

#define DEFAULT_ACCEPTMBOX_SIZE (TCPIP_MBOX_SIZE)

/* FreeRTOS specific config options */
#define LWIP_FREERTOS_CHECK_CORE_LOCKING 1
#define LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX 1
#define LWIP_FREERTOS_SYS_ARCH_PROTECT_SANITY_CHECK 1
#define LWIP_FREERTOS_CHECK_QUEUE_EMPTY_ON_FREE 1

#ifdef __clang__
/*---------------------------------------------------------------*/
#if defined(__ARM_ARCH) && (defined(__TI_EABI__) || defined(__clang__))
/*------------------------------------------------------------------------*/
/* Under EABI, use function to access errno since it likely has TLS in    */
/* a thread-safe version of the RTS library.                              */
/*------------------------------------------------------------------------*/
extern volatile int* __aeabi_errno_addr(void);
// #define errno (*__aeabi_errno_addr())
#elif !defined(__C6X_MIGRATION__) && defined(__TMS320C6X__) && defined(__TI_EABI__)
/*------------------------------------------------------------------------*/
/* Under EABI, use function to access errno since it likely has TLS in    */
/* a thread-safe version of the RTS library.                              */
/*------------------------------------------------------------------------*/
extern int* __c6xabi_errno_addr(void);
__TI_TLS_DATA_DECL(int, __errno);

#define errno (*__c6xabi_errno_addr())
#else
extern _DATA_ACCESS int errno;
_TI_PROPRIETARY_PRAGMA("diag_push")
/* errno is not allowed under MISRA, anyway */
_TI_PROPRIETARY_PRAGMA("CHECK_MISRA(\"-5.6\")")  /* duplicated name in another scope (errno) */
_TI_PROPRIETARY_PRAGMA("CHECK_MISRA(\"-19.4\")") /* macro expands to parenthesized */
#define errno errno
_TI_PROPRIETARY_PRAGMA("diag_pop")
#endif
#endif
///** ETH_PAD_SIZE: number of bytes added before the ethernet header to ensure
//* alignment of payload after that header. Since the header is 14 bytes long,
//* without this padding e.g. addresses in the IP header will not be aligned
//* on a 32-bit boundary, so setting this to 2 can speed up 32-bit-platforms.
//*/
// #if !defined ETH_PAD_SIZE || defined __DOXYGEN__
//    #define ETH_PAD_SIZE                    2
// #endif

#endif /* LWIP_LWIPOPTS_H */
