#include "services/network_instantiations/SocketAddress.hpp"
#include <cstring>

namespace services::detail
{
    int AddressFamily(IPVersions versions)
    {
        return versions == IPVersions::ipv6 ? AF_INET6 : AF_INET;
    }

    int AddressFamily(const UdpSocket& socket)
    {
        return std::holds_alternative<Udpv6Socket>(socket) ? AF_INET6 : AF_INET;
    }

    int AddressFamily(const IPAddress& address)
    {
        return std::holds_alternative<IPv6Address>(address) ? AF_INET6 : AF_INET;
    }

    UdpSocket AnyAddressSocket(int family, uint16_t port)
    {
        if (family == AF_INET6)
            return MakeUdpSocket(IPv6Address{}, port);
        return MakeUdpSocket(IPv4Address{}, port);
    }

    socklen_t FillSocketAddress(const UdpSocket& socket, sockaddr_storage& address)
    {
        std::memset(&address, 0, sizeof(address));

        if (std::holds_alternative<Udpv4Socket>(socket))
        {
            auto& address4 = reinterpret_cast<sockaddr_in&>(address);
            const auto& socket4 = std::get<Udpv4Socket>(socket);
            address4.sin_family = AF_INET;
            address4.sin_addr.s_addr = htonl(ConvertToUint32(socket4.first));
            address4.sin_port = htons(socket4.second);
            return sizeof(sockaddr_in);
        }

        auto& address6 = reinterpret_cast<sockaddr_in6&>(address);
        const auto& socket6 = std::get<Udpv6Socket>(socket);
        address6.sin6_family = AF_INET6;
        auto networkOrder = ToNetworkOrder(socket6.first);
        std::memcpy(&address6.sin6_addr, networkOrder.data(), sizeof(address6.sin6_addr));
        address6.sin6_port = htons(socket6.second);
        return sizeof(sockaddr_in6);
    }
}
