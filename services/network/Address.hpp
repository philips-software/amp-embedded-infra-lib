#ifndef SERVICES_ADDRESS_HPP
#define SERVICES_ADDRESS_HPP

#include "infra/stream/OutputStream.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/Variant.hpp"
#include <array>
#include <utility>

namespace services
{
    using IPv4Address = std::array<uint8_t, 4>;
    using IPv6Address = std::array<uint16_t, 8>;
    using IPv6AddressNetworkOrder = std::array<infra::BigEndian<uint16_t>, 8>;
    using IPAddress = infra::Variant<services::IPv4Address, services::IPv6Address>;

    enum class IPVersions
    {
        ipv4,
        ipv6,
        both
    };

    using Udpv4Socket = std::pair<IPv4Address, uint16_t>;
    using Udpv6Socket = std::pair<IPv6Address, uint16_t>;
    using UdpSocket = infra::Variant<Udpv4Socket, Udpv6Socket>;

    UdpSocket MakeUdpSocket(IPAddress address, uint16_t port);

    IPAddress GetAddress(UdpSocket socket);
    uint16_t GetPort(UdpSocket socket);
    IPVersions GetVersion(IPAddress socket);
    IPVersions GetVersion(UdpSocket socket);

    IPv4Address IPv4AddressLocalHost();
    IPv6Address IPv6AddressLocalHost();

    uint32_t ConvertToUint32(IPv4Address address);
    IPv4Address ConvertFromUint32(uint32_t address);

    infra::Optional<IPAddress> ParseIpAddress(infra::BoundedConstString address);
    infra::Optional<IPv4Address> ParseIpv4Address(infra::BoundedConstString address);
    infra::Optional<IPv6Address> ParseFullIpv6Address(infra::BoundedConstString address);

    IPv6AddressNetworkOrder ToNetworkOrder(IPv6Address address);

    struct IPv4InterfaceAddresses
    {
        IPv4Address address;
        IPv4Address netmask;
        IPv4Address gateway;

        bool operator==(const IPv4InterfaceAddresses& other) const;
        bool operator!=(const IPv4InterfaceAddresses& other) const;
    };

    struct Ipv4Config
    {
        bool useDhcp;
        IPv4InterfaceAddresses staticAddresses;

        bool operator==(const Ipv4Config& other) const;
        bool operator!=(const Ipv4Config& other) const;
    };

    class IPv4Info
    {
    public:
        IPv4Info() = default;
        IPv4Info(const IPv4Info& other) = delete;
        IPv4Info& operator=(const IPv4Info& other) = delete;
        ~IPv4Info() = default;

        virtual IPv4Address GetIPv4Address() const = 0;
        virtual IPv4InterfaceAddresses GetIPv4InterfaceAddresses() const = 0;
    };

    class IPv6Info
    {
    public:
        IPv6Info() = default;
        IPv6Info(const IPv6Info& other) = delete;
        IPv6Info& operator=(const IPv6Info& other) = delete;
        ~IPv6Info() = default;

        virtual IPv6Address LinkLocalAddress() const = 0;
    };

    class IPInfo
        : public IPv4Info
        , public IPv6Info
    {};
}

namespace infra
{
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::IPv4Address& address);
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::IPv6Address& address);
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::IPAddress& address);
}

#endif
