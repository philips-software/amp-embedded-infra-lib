#include "services/network/DnsServer.hpp"
#include "infra/util/EnumCast.hpp"

namespace
{
    // Since we repeat the question in the answer we can use a pointer to the name
    // in the answer record. The name starts after the DnsRecordHeader.
    static const std::array<uint8_t, 2> rnameCompression{ 0xC0, sizeof(services::DnsRecordHeader) };
}

namespace services
{
    DnsServer::DnsServer(services::DatagramFactory& factory, services::IPv4Info& ipv4Info)
        : datagramExchange(factory.Listen(*this, dnsPort, IPVersions::ipv4))
        , ipv4Info(ipv4Info)
    {}

    void DnsServer::DataReceived(infra::StreamReaderWithRewinding& reader, services::UdpSocket from)
    {
        if (question)
            return;

        question.Emplace(reader);
        if (question->IsValid() && question->RequestIncludesOneQuestion())
            datagramExchange->RequestSendStream(AnswerSize(), from);
        else
            question = infra::none;
    }

    void DnsServer::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        assert(question != infra::none);

        infra::DataOutputStream::WithErrorPolicy stream(*writer);

        DnsRecordHeader header{ question->QueryId(), DnsRecordHeader::flagsResponse, 1, 1, 0, 0 };
        DnsHostnamePartsString hostnameParts{ question->Hostname() };
        DnsQuestionFooter questionFooter{ DnsType::dnsTypeA, DnsClass::dnsClassIn };
        DnsRecordPayload payload{ DnsType::dnsTypeA, DnsClass::dnsClassIn, std::chrono::seconds(60), sizeof(IPv4Address) };

        stream << header;
        stream << hostnameParts;
        stream << questionFooter;
        stream << rnameCompression;
        stream << payload;
        stream << ipv4Info.GetIPv4Address();

        question = infra::none;
    }

    std::size_t DnsServer::AnswerSize() const
    {
        assert(question != infra::none);

        return sizeof(DnsRecordHeader) + question->QueryNameSize() + sizeof(DnsQuestionFooter) + sizeof(rnameCompression) + sizeof(DnsRecordPayload) + sizeof(IPv4Address);
    }

    DnsServer::QuestionParser::QuestionParser(infra::StreamReaderWithRewinding& reader)
        : reader(reader)
    {
        stream >> header;
        ReconstructHostname();
        stream.Consume(QueryNameSize());
        stream >> footer;
    }

    bool DnsServer::QuestionParser::RequestIncludesOneQuestion() const
    {
        return header.questionsCount == static_cast<uint16_t>(1u) &&
            header.answersCount == static_cast<uint16_t>(0u) &&
            header.nameServersCount == static_cast<uint16_t>(0u) &&
            header.additionalRecordsCount == static_cast<uint16_t>(0u);
    }

    bool DnsServer::QuestionParser::IsValid() const
    {
        if (stream.Failed())
            return false;

        if ((header.flags & DnsRecordHeader::flagsOpcodeMask) != DnsRecordHeader::flagsOpcodeQuery)
            return false;

        if ((header.flags & DnsRecordHeader::flagsResponse) != DnsRecordHeader::flagsQuery)
            return false;

        if (footer.type != infra::enum_cast(DnsType::dnsTypeA))
            return false;

        if (footer.class_ != infra::enum_cast(DnsClass::dnsClassIn))
            return false;

        return true;
    }

    std::size_t DnsServer::QuestionParser::QueryNameSize() const
    {
        if (reconstructedHostname.empty())
            return 1;

        // After breaking-up the reconstructed hostname in parts the total size
        // can be replaced by:
        // size + (parts + dots replaced by size) + terminating zero
        return reconstructedHostname.size() + 2;
    }

    uint16_t DnsServer::QuestionParser::QueryId() const
    {
        return header.id;
    }

    infra::BoundedConstString DnsServer::QuestionParser::Hostname() const
    {
        return reconstructedHostname;
    }

    void DnsServer::QuestionParser::ReconstructHostname()
    {
        auto hostnameStart = reader.ConstructSaveMarker();
        DnsHostnamePartsStream hostnameParts(reader, hostnameStart);

        while (!hostnameParts.Empty())
        {
            reconstructedHostname += hostnameParts.Current();
            hostnameParts.ConsumeCurrent();

            if (!hostnameParts.Empty())
                reconstructedHostname += '.';
        }
    }
}
