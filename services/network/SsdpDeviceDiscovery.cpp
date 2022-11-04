#include "services/network/SsdpDeviceDiscovery.hpp"
#include "infra/stream/StringInputStream.hpp"

namespace services
{
    namespace
    {
        const uint16_t ssdpPort = 1900;
        const IPv4Address ssdpMulticastAddressIpv4 = { 239, 255, 255, 250 };
        const IPv6Address ssdpMulticastAddressIpv6 = { 0xff02, 0, 0, 0, 0, 0, 0, 0x0c };
    }

    SsdpDeviceDiscovery::SsdpDeviceDiscovery(DatagramFactory& datagramFactory, Multicast& multicast)
        : multicast(multicast)
        , datagramExchange(datagramFactory.Listen(*this, ssdpPort, IPVersions::both))
        , activeDiscovery(*this, datagramFactory)
    {
        multicast.JoinMulticastGroup(datagramExchange, ssdpMulticastAddressIpv4);
        multicast.JoinMulticastGroup(datagramExchange, ssdpMulticastAddressIpv6);
    }

    SsdpDeviceDiscovery::~SsdpDeviceDiscovery()
    {
        multicast.LeaveMulticastGroup(datagramExchange, ssdpMulticastAddressIpv4);
        multicast.LeaveMulticastGroup(datagramExchange, ssdpMulticastAddressIpv6);
    }

    void SsdpDeviceDiscovery::Discover(infra::BoundedConstString searchTarget, uint8_t maxWaitResponseTime, IPVersions ipVersion)
    {
        this->searchTarget = searchTarget;
        activeDiscovery.Discover(maxWaitResponseTime, ipVersion);
    }

    void SsdpDeviceDiscovery::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, services::UdpSocket from)
    {
        bool skipped = SkippedNotify(*reader);

        ResponseValidator validator(*reader, searchTarget, skipped);
    }

    void SsdpDeviceDiscovery::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        std::abort();
    }

    bool SsdpDeviceDiscovery::SkippedNotify(infra::StreamReaderWithRewinding& reader) const
    {
        static const infra::BoundedConstString notify = "NOTIFY * HTTP/1.1\r\n";

        infra::BoundedString::WithStorage<20> checkNotify(notify.size(), 0);
        infra::TextInputStream::WithErrorPolicy stream(reader, infra::noFail);
        stream >> checkNotify;

        if (checkNotify != notify)
        {
            reader.Rewind(0);
            return false;
        }

        return true;
    }

    SsdpDeviceDiscovery::ResponseValidator::ResponseValidator(infra::StreamReaderWithRewinding& reader, infra::BoundedConstString searchTarget, bool skippedStatus)
        : headerParser(*this)
        , searchTarget(searchTarget)
    {
        if (skippedStatus)
        {
            infra::StringInputStreamReader statusReader("HTTP/1.1 200 OK\r\n");
            headerParser.DataReceived(statusReader);
        }

        headerParser.DataReceived(reader);
    }

    bool SsdpDeviceDiscovery::ResponseValidator::ValidResponse() const
    {
        return valid && foundDevice && !error && done;
    }

    void SsdpDeviceDiscovery::ResponseValidator::StatusAvailable(services::HttpStatusCode code, infra::BoundedConstString statusLine)
    {
        valid &= code == services::HttpStatusCode::OK;
    }

    void SsdpDeviceDiscovery::ResponseValidator::HeaderAvailable(services::HttpHeader header)
    {
        if ((infra::CaseInsensitiveCompare(header.Field(), "ST") || infra::CaseInsensitiveCompare(header.Field(), "NT")) && header.Value() == searchTarget)
            foundDevice = true;
    }

    void SsdpDeviceDiscovery::ResponseValidator::HeaderParsingDone(bool error)
    {
        done = true;
        this->error = error;
    }

    SsdpDeviceDiscovery::ActiveDiscovery::ActiveDiscovery(SsdpDeviceDiscovery& discovery, services::DatagramFactory& datagramFactory)
        : discovery(discovery)
        , discoverExchange(datagramFactory.Listen(*this))
    {}

    void SsdpDeviceDiscovery::ActiveDiscovery::Discover(uint8_t maxWaitResponseTime, IPVersions ipVersion)
    {
        this->ipVersion = ipVersion;
        this->maxWaitResponseTime = maxWaitResponseTime;

        discoverExchange->RequestSendStream(200, MakeSsdpUdpSocket(ipVersion));
    }

    void SsdpDeviceDiscovery::ActiveDiscovery::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, services::UdpSocket from)
    {
        discovery.DataReceived(std::move(reader), from);
    }

    void SsdpDeviceDiscovery::ActiveDiscovery::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::TextOutputStream::WithErrorPolicy stream(*writer);

        stream
            << "M-SEARCH * HTTP/1.1\r\n"
            << "HOST: " << AsCanonicalFormIp(GetSsdpMulticastAddress(ipVersion)) << ":" << ssdpPort << "\r\n"
            << "MAN: \"ssdp:discover\"\r\n"
            << "ST: " << discovery.searchTarget << "\r\n"
            << "MX: " << maxWaitResponseTime << "\r\n";
    }

    UdpSocket SsdpDeviceDiscovery::ActiveDiscovery::MakeSsdpUdpSocket(IPVersions ipVersion)
    {
        switch (ipVersion)
        {
            case IPVersions::ipv4:
                return Udpv4Socket{ ssdpMulticastAddressIpv4, ssdpPort };
            case IPVersions::ipv6:
                return Udpv6Socket{ ssdpMulticastAddressIpv6, ssdpPort };
            default:
                std::abort();
        }
    }

    IPAddress SsdpDeviceDiscovery::ActiveDiscovery::GetSsdpMulticastAddress(IPVersions ipVersion)
    {
        switch (ipVersion)
        {
            case IPVersions::ipv4:
                return ssdpMulticastAddressIpv4;
            case IPVersions::ipv6:
                return ssdpMulticastAddressIpv6;
            default:
                std::abort();
        }
    }
}
