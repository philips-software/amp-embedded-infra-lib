#ifndef SERVICES_SSDP_DEVICE_DISCOVERY_HPP
#define SERVICES_SSDP_DEVICE_DISCOVERY_HPP

#include "services/network/Datagram.hpp"
#include "services/network/Multicast.hpp"
#include "services/network/Http.hpp"

namespace services
{
    class SsdpDeviceDiscovery;

    class SsdpResponseObserver
        : public infra::SingleObserver<SsdpResponseObserver, SsdpDeviceDiscovery>
    {
    public:
        using SingleObserver<SsdpResponseObserver, SsdpDeviceDiscovery>::SingleObserver;
    
        virtual void Response(IPAddress& address) = 0;
    };

    class SsdpDeviceDiscovery
        : private DatagramExchangeObserver
        , public infra::Subject<SsdpResponseObserver>
    {
    public:
        SsdpDeviceDiscovery(DatagramFactory& datagramFactory, Multicast& multicast);
        ~SsdpDeviceDiscovery();

        void Discover(infra::BoundedConstString searchTarget, uint8_t maxWaitResponseTime, IPVersions ipVersion = IPVersions::ipv4);

    private:
        // Implementation of DatagramExchangeObserver
        virtual void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, services::UdpSocket from) override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        bool SkippedNotify(infra::StreamReaderWithRewinding& reader) const;

    private:
        class ResponseValidator
            : private services::HttpHeaderParserObserver
        {
        public:
            ResponseValidator(infra::StreamReaderWithRewinding& reader, infra::BoundedConstString searchTarget, bool skippedStatus);

            bool ValidResponse() const;

        private:
            // Implementation of HttpHeaderParserObserver
            virtual void StatusAvailable(services::HttpStatusCode code, infra::BoundedConstString statusLine) override;
            virtual void HeaderAvailable(services::HttpHeader header) override;
            virtual void HeaderParsingDone(bool error) override;

        private:
            bool valid = true;
            bool foundDevice = false;
            bool done = false;
            bool error = false;
            HttpHeaderParser headerParser;
            infra::BoundedConstString searchTarget;
        };

        class ActiveDiscovery
            : private services::DatagramExchangeObserver
        {
        public:
            ActiveDiscovery(SsdpDeviceDiscovery& discovery, services::DatagramFactory& datagramFactory);

            void Discover(uint8_t maxWaitResponseTime, IPVersions ipVersion = IPVersions::ipv4);

        private:
            // Implementation of DatagramExchangeObserver
            virtual void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, services::UdpSocket from) override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

            UdpSocket MakeSsdpUdpSocket(IPVersions ipVersion);
            IPAddress GetSsdpMulticastAddress(IPVersions ipVersion);

        private:
            SsdpDeviceDiscovery& discovery;
            infra::SharedPtr<services::DatagramExchange> discoverExchange;
            IPVersions ipVersion;
            uint8_t maxWaitResponseTime;
        };
    
    private:
        DatagramFactory& datagramFactory;
        Multicast& multicast;
        infra::SharedPtr<DatagramExchange> datagramExchange;
        ActiveDiscovery activeDiscovery;
        infra::BoundedConstString searchTarget;
    };
}

#endif
