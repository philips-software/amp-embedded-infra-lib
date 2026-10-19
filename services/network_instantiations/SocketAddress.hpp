#ifndef SERVICES_SOCKET_ADDRESS_HPP
#define SERVICES_SOCKET_ADDRESS_HPP

#include "services/network/Address.hpp"

#ifdef EMIL_NETWORK_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef EMIL_NETWORK_BSD
#include <netinet/in.h>
#include <sys/socket.h>
#endif

namespace services::detail
{
    int AddressFamily(IPVersions versions);
    int AddressFamily(const UdpSocket& socket);
    int AddressFamily(const IPAddress& address);
    UdpSocket AnyAddressSocket(int family, uint16_t port);
    socklen_t FillSocketAddress(const UdpSocket& socket, sockaddr_storage& address);
}

#endif
