#include "infra/util/CompareMembers.hpp"
#include "services/network/Address.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
    UdpSocket MakeUdpSocket(IPAddress address, uint16_t port)
    {
        if (address.Is<services::IPv4Address>())
            return Udpv4Socket{ address.Get<services::IPv4Address>(), port };
        else
            return Udpv6Socket{ address.Get<services::IPv6Address>(), port };
    }

    IPAddress GetAddress(UdpSocket socket)
    {
        if (socket.Is<Udpv4Socket>())
            return socket.Get<Udpv4Socket>().first;
        else
            return socket.Get<Udpv6Socket>().first;
    }

    uint16_t GetPort(UdpSocket socket)
    {
        if (socket.Is<Udpv4Socket>())
            return socket.Get<Udpv4Socket>().second;
        else
            return socket.Get<Udpv6Socket>().second;
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
        return services::IPv4Address{
            static_cast<uint8_t>(address >> 24),
            static_cast<uint8_t>(address >> 16),
            static_cast<uint8_t>(address >> 8),
            static_cast<uint8_t>(address)
        };
    }

    infra::Optional<IPAddress> ParseIpAddress(infra::BoundedConstString address)
    {
        auto ipv4 = ParseIpv4Address(address);
        if (ipv4)
            return infra::MakeOptional(IPAddress{ *ipv4 });

        auto ipv6 = ParseFullIpv6Address(address);
        if (ipv6)
            return infra::MakeOptional(IPAddress{ *ipv6 });

        return infra::none;
    }

    infra::Optional<IPv4Address> ParseIpv4Address(infra::BoundedConstString address)
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
                return infra::none;

            if (decimal != static_cast<uint8_t>(decimal))
                return infra::none;

            parsedCount += token.size();
            ipv4Address[i] = static_cast<uint8_t>(decimal);
        }

        if (parsedCount + 3 != address.size())
            return infra::none;

        return infra::MakeOptional(ipv4Address);
    }

    infra::Optional<IPv6Address> ParseFullIpv6Address(infra::BoundedConstString address)
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
                return infra::none;

            parsedCount += token.size();
            ipv6Address[i] = decimal;
        }

        if (parsedCount + 7 != address.size())
            return infra::none;

        return infra::MakeOptional(ipv6Address);
    }

    IPv6AddressNetworkOrder ToNetworkOrder(IPv6Address address)
    {
        IPv6AddressNetworkOrder result;
        std::copy(address.begin(), address.end(), result.begin());
        return result;
    }

    bool IPv4InterfaceAddresses::operator==(const IPv4InterfaceAddresses& other) const
    {
        return infra::Equals()
            (address, other.address)
            (netmask, other.netmask)
            (gateway, other.gateway);
    }

    bool IPv4InterfaceAddresses::operator!=(const IPv4InterfaceAddresses& other) const
    {
        return !(*this == other);
    }

    bool Ipv4Config::operator==(const Ipv4Config& other) const
    {
        return infra::Equals()
            (useDhcp, other.useDhcp)
            (staticAddresses, other.staticAddresses);
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

    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::IPAddress& address)
    {
        if (address.Is<services::IPv4Address>())
            stream << address.Get<services::IPv4Address>();
        else
            stream << address.Get<services::IPv6Address>();

        return stream;
    }
}
