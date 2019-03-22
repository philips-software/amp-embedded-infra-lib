#ifndef SERVICES_LLMNR_RESPONDER_HPP
#define SERVICES_LLMNR_RESPONDER_HPP

#include "infra/util/Endian.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Multicast.hpp"

namespace services
{
    class LlmnrResponder
        : public services::DatagramExchangeObserver
    {
    public:
        static const services::IPv4Address llmnpMulticastAddress;
        static const uint16_t llmnpPort = 5355;

        LlmnrResponder(services::DatagramFactory& factory, services::Multicast& multicast, services::IPv4Info& ipv4Info, infra::BoundedConstString name);
        ~LlmnrResponder();

        virtual void DataReceived(infra::StreamReader& reader, services::UdpSocket from) override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        struct Header
        {
            static const uint16_t flagsQuery = 0;
            static const uint16_t flagsResponse = 0x8000;

            infra::BigEndian<uint16_t> id;
            infra::BigEndian<uint16_t> flags;
            infra::BigEndian<uint16_t> questionsCount;
            infra::BigEndian<uint16_t> answersCount;
            infra::BigEndian<uint16_t> nameServersCount;
            infra::BigEndian<uint16_t> additionalRecordsCount;
        };

        struct Footer
        {
            static const uint16_t typeA = 1;
            static const uint16_t classIn = 1;

            infra::BigEndian<uint16_t> type = typeA;
            infra::BigEndian<uint16_t> class_ = classIn;
        };

        struct Answer
        {
            static const uint16_t nameIsPointer = 0xc000;
            static const uint16_t defaultTtl = 30;  // RFC4795 recommends a TTL of 30 seconds

            infra::BigEndian<uint16_t> name;
            infra::BigEndian<uint16_t> type;
            infra::BigEndian<uint16_t> class_;
            infra::BigEndian<uint16_t> ttl1;
            infra::BigEndian<uint16_t> ttl2;
            infra::BigEndian<uint16_t> dataLength;
            services::IPv4Address address;
        };

    private:
        bool PacketValid(const Header& header, const Footer& footer) const;

    private:
        infra::SharedPtr<services::DatagramExchange> datagramExchange;
        services::Multicast& multicast;
        services::IPv4Info& ipv4Info;
        infra::BoundedConstString name;
        bool replying = false;
        infra::BigEndian<uint16_t> id;
    };
}

#endif
