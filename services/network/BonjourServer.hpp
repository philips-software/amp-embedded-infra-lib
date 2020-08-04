#ifndef SERVICES_BONJOUR_SERVER_HPP
#define SERVICES_BONJOUR_SERVER_HPP

#include "infra/stream/CountingOutputStream.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Dns.hpp"
#include "services/network/Multicast.hpp"

namespace services
{
    class BonjourServer
        : public DatagramExchangeObserver
    {
    public:
        BonjourServer(DatagramFactory& factory, Multicast& multicast, infra::BoundedConstString instance, infra::BoundedConstString serviceName, infra::BoundedConstString type,
            infra::Optional<IPv4Address> ipv4Address, infra::Optional<IPv6Address> ipv6Address, uint16_t port, const DnsHostnameParts& text);
        ~BonjourServer();

        virtual void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from) override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        class Answer
        {
        public:
            Answer(BonjourServer& server, uint16_t queryId, infra::StreamWriter& writer, uint16_t answersCount, uint16_t additionalRecordsCount);

            uint16_t Answers() const;
            uint16_t AdditionalRecords() const;

            void AddAAnswer();
            void AddAaaaAnswer();
            void AddPtrAnswer();
            void AddSrvAnswer();
            void AddTxtAnswer();

            void AddAAdditional();
            void AddAaaaAdditional();
            void AddSrvAdditional();
            void AddTxtAdditional();

        private:
            void AddA(const DnsHostnameParts& dnsHostname);
            void AddAaaa(const DnsHostnameParts& dnsHostname);
            void AddPtr(const DnsHostnameParts& dnsHostname);
            void AddSrv(const DnsHostnameParts& dnsHostname);
            void AddTxt(const DnsHostnameParts& dnsHostname);

        private:
            BonjourServer& server;
            uint16_t queryId;
            infra::StreamWriter& writer;
            infra::DataOutputStream::WithErrorPolicy stream{ writer };

            uint16_t answersCount = 0;
            uint16_t additionalRecordsCount = 0;
        };

        class QuestionParser
        {
        public:
            QuestionParser(BonjourServer& server, infra::StreamReaderWithRewinding& reader);
            QuestionParser(BonjourServer& server, infra::StreamReaderWithRewinding& reader, infra::StreamWriter& writer);

            bool HasAnswer() const;
            void RequestSendStream(DatagramExchange& datagramExchange, UdpSocket from);

        private:
            void CreateAnswer(infra::StreamWriter& writer);
            bool IsValidQuestion() const;
            void ReadQuestion();
            void ReadQuestionForAdditionalRecords();
            bool IsFooterValid() const;
            bool IsQueryForMe() const;
            bool MyFullInstanceName() const;
            bool MyShortInstanceName() const;
            bool MyServiceName() const;

            void ReadHostname();
            bool HostNameIs(infra::MemoryRange<infra::BoundedConstString> components) const;

        private:
            BonjourServer& server;
            infra::StreamReaderWithRewinding& reader;
            infra::DataInputStream::WithErrorPolicy stream{ reader, infra::noFail };
            std::size_t startMarker{ reader.ConstructSaveMarker() };
            infra::CountingStreamWriter countingWriter;
            DnsRecordHeader header{};
            infra::BoundedString::WithStorage<253> reconstructedHostname;
            DnsQuestionFooter footer{};
            bool valid = true;

            infra::Optional<Answer> answer;
            uint16_t answersCount = 0;
            uint16_t additionalRecordsCount = 0;
        };

    private:
        infra::SharedPtr<DatagramExchange> datagramExchange;
        Multicast& multicast;
        infra::BoundedConstString instance;
        infra::BoundedConstString serviceName;
        infra::BoundedConstString type;
        infra::Optional<IPv4Address> ipv4Address;
        infra::Optional<IPv6Address> ipv6Address;
        uint16_t port;
        const DnsHostnameParts& text;
        infra::SharedPtr<infra::StreamReaderWithRewinding> waitingReader;
    };
}

#endif
