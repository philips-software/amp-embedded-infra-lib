#include "services/network/Address.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/util/CompareMembers.hpp"
#include "infra/util/Overloaded.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
    namespace
    {
        UdpSocket ToUdpSocket(const IPv4Address& address, uint16_t port)
        {
            return Udpv4Socket{ address, port };
        }

        UdpSocket ToUdpSocket(const IPv6Address& address, uint16_t port)
        {
            return Udpv6Socket{ address, port };
        }

        IPAddress ToAddress(const Udpv4Socket& socket)
        {
            return IPv4Address{ socket.first };
        }

        IPAddress ToAddress(const Udpv6Socket& socket)
        {
            return IPv6Address{ socket.first };
        }

        IPVersions ToIpVersion(const IPv4Address&)
        {
            return IPVersions::ipv4;
        }

        IPVersions ToIpVersion(const IPv6Address&)
        {
            return IPVersions::ipv6;
        }

        IPVersions ToIpVersion(const Udpv4Socket&)
        {
            return IPVersions::ipv4;
        }

        IPVersions ToIpVersion(const Udpv6Socket&)
        {
            return IPVersions::ipv6;
        }
    }

    UdpSocket MakeUdpSocket(IPAddress address, uint16_t port)
    {
        return std::visit([&port](const auto& address)
            {
                return ToUdpSocket(address, port);
            },
            address);
    }

    IPAddress GetAddress(UdpSocket socket)
    {
        return std::visit([](const auto& socket)
            {
                return ToAddress(socket);
            },
            socket);
    }

    uint16_t GetPort(UdpSocket socket)
    {
        return std::visit([](const auto& socket)
            {
                return socket.second;
            },
            socket);
    }

    IPVersions GetVersion(IPAddress address)
    {
        return std::visit([](const auto& address)
            {
                return ToIpVersion(address);
            },
            address);
    }

    IPVersions GetVersion(UdpSocket socket)
    {
        return std::visit([](const auto& socket)
            {
                return ToIpVersion(socket);
            },
            socket);
    }

    IPv4Address IPv4AddressLocalHost()
    {
        return IPv4Address{ 127, 0, 0, 1 };
    }

    IPv6Address IPv6AddressLocalHost()
    {
        return IPv6Address{ 0, 0, 0, 0, 0, 0, 0, 1 };
    }

    uint32_t ConvertToUint32(IPv4Address address)
    {
        return address[3] | (address[2] << 8) | (address[1] << 16) | (address[0] << 24);
    }

    IPv4Address ConvertFromUint32(uint32_t address)
    {
        return IPv4Address{
            static_cast<uint8_t>(address >> 24),
            static_cast<uint8_t>(address >> 16),
            static_cast<uint8_t>(address >> 8),
            static_cast<uint8_t>(address)
        };
    }

    std::optional<IPAddress> ParseIpAddress(infra::BoundedConstString address)
    {
        auto ipv4 = ParseIpv4Address(address);
        if (ipv4)
            return std::make_optional(IPAddress{ *ipv4 });

        auto ipv6 = ParseFullIpv6Address(address);
        if (ipv6)
            return std::make_optional(IPAddress{ *ipv6 });

        return std::nullopt;
    }

    std::optional<IPv4Address> ParseIpv4Address(infra::BoundedConstString address)
    {
        IPv4Address ipv4Address;
        std::size_t parsedCount = 0;
        infra::Tokenizer tokenizer(address, '.');

        for (int i = 0; i < 4; i++)
        {
            auto token = tokenizer.Token(i);

            uint16_t decimal;
            infra::StringInputStream stream(token, infra::softFail);
            stream >> infra::Width(3) >> decimal;

            if (stream.Failed() || stream.Available() != 0)
                return std::nullopt;

            if (decimal != static_cast<uint8_t>(decimal))
                return std::nullopt;

            parsedCount += token.size();
            ipv4Address[i] = static_cast<uint8_t>(decimal);
        }

        if (parsedCount + 3 != address.size())
            return std::nullopt;

        return std::make_optional(ipv4Address);
    }

    std::optional<IPv6Address> ParseFullIpv6Address(infra::BoundedConstString address)
    {
        IPv6Address ipv6Address;
        std::size_t parsedCount = 0;
        infra::Tokenizer tokenizer(address, ':');

        for (int i = 0; i < 8; i++)
        {
            auto token = tokenizer.Token(i);

            uint32_t decimal;
            infra::StringInputStream stream(token, infra::softFail);
            stream >> infra::hex >> infra::Width(4) >> decimal;

            if (stream.Failed() || stream.Available() != 0)
                return std::nullopt;

            parsedCount += token.size();
            ipv6Address[i] = decimal;
        }

        if (parsedCount + 7 != address.size())
            return std::nullopt;

        return std::make_optional(ipv6Address);
    }

    IPv6Address FromNetworkOrder(IPv6AddressNetworkOrder address)
    {
        IPv6Address result;
        std::copy(address.begin(), address.end(), result.begin());
        return result;
    }

    IPv6AddressNetworkOrder ToNetworkOrder(IPv6Address address)
    {
        IPv6AddressNetworkOrder result;
        std::copy(address.begin(), address.end(), result.begin());
        return result;
    }

    bool IPv4InterfaceAddresses::operator==(const IPv4InterfaceAddresses& other) const
    {
        return infra::Equals()(address, other.address)(netmask, other.netmask)(gateway, other.gateway);
    }

    bool IPv4InterfaceAddresses::operator!=(const IPv4InterfaceAddresses& other) const
    {
        return !(*this == other);
    }

    bool Ipv4Config::operator==(const Ipv4Config& other) const
    {
        return infra::Equals()(useDhcp, other.useDhcp)(addresses, other.addresses);
    }

    bool Ipv4Config::operator!=(const Ipv4Config& other) const
    {
        return !(*this == other);
    }
}

namespace infra
{
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::IPv4Address& address)
    {
        stream << address[0] << "." << address[1] << "." << address[2] << "." << address[3];

        return stream;
    }

    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::IPv6Address& address)
    {
        stream << infra::hex << address[0] << ":" << address[1] << ":" << address[2] << ":" << address[3] << ":" << address[4] << ":" << address[5] << ":" << address[6] << ":" << address[7];

        return stream;
    }

    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::IPv6AddressNetworkOrder& address)
    {
        stream << infra::hex << address[0] << ":" << address[1] << ":" << address[2] << ":" << address[3] << ":" << address[4] << ":" << address[5] << ":" << address[6] << ":" << address[7];

        return stream;
    }

    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::IPAddress& address)
    {
        std::visit([&stream](const auto& address)
            {
                stream << address;
            },
            address);

        return stream;
    }

    AsCanonicalFormIpHelper::AsCanonicalFormIpHelper(const services::IPAddress& address)
        : address(address)
    {}

    TextOutputStream& operator<<(TextOutputStream& stream, const AsCanonicalFormIpHelper& asCanonicalFormIpHelper)
    {
        const auto visitor = infra::Overloaded{
            [&stream](const services::IPv4Address& address)
            {
                stream << address;
            },
            [&stream](const services::IPv6Address& address)
            {
                stream << "[" << address << "]";
            },
        };

        std::visit(visitor, asCanonicalFormIpHelper.address);

        return stream;
    }

    AsCanonicalFormIpHelper AsCanonicalFormIp(const services::IPAddress& address)
    {
        return AsCanonicalFormIpHelper(address);
    }
}
