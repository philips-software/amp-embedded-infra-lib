#ifndef SERVICES_DNS_SERVER_HPP
#define SERVICES_DNS_SERVER_HPP

#include "services/network/Datagram.hpp"
#include "services/network/Dns.hpp"

namespace services
{
    class DnsServer
        : public services::DatagramExchangeObserver
    {
    public:
        static const uint16_t dnsPort = 53;

        using DnsEntry = std::pair<infra::BoundedConstString, services::IPAddress>;

        struct DnsEntries
        {
            infra::MemoryRange<const DnsEntry> entries;
        };

        DnsServer(services::DatagramFactory& factory, DnsEntries dnsEntries);

        virtual void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, services::UdpSocket from) override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    protected:
        virtual infra::Optional<DnsEntry> FindAnswer(infra::BoundedConstString hostname) = 0;

        infra::MemoryRange<const DnsEntry> Entries() const;

    private:
        std::size_t AnswerSize() const;

    private:
        class QuestionParser
        {
        public:
            explicit QuestionParser(infra::StreamReaderWithRewinding& reader);

            bool RequestIncludesOneQuestion() const;
            bool IsValid() const;

            std::size_t QueryNameSize() const;
            uint16_t QueryId() const;
            infra::BoundedConstString Hostname() const;

        private:
            void ReconstructHostname();

        private:
            infra::StreamReaderWithRewinding& reader;
            infra::DataInputStream::WithErrorPolicy stream{ reader, infra::noFail };
            DnsRecordHeader header{};
            infra::BoundedString::WithStorage<253> reconstructedHostname;
            DnsQuestionFooter footer{};
        };

    private:
        infra::SharedPtr<services::DatagramExchange> datagramExchange;
        infra::MemoryRange<const DnsEntry> dnsEntries;
        infra::Optional<QuestionParser> question;
        infra::Optional<DnsEntry> answer;
    };

    class DnsServerImpl
        : public DnsServer
    {
    public:
        using DnsServer::DnsServer;

    protected:
        virtual infra::Optional<DnsEntry> FindAnswer(infra::BoundedConstString hostname) override;
    };
}

#endif
