#include "services/network/BonjourServer.hpp"
#include "infra/util/BitLogic.hpp"
#include "infra/util/EnumCast.hpp"

namespace services
{
    namespace
    {
        const uint16_t mdnsPort = 5353;
        const IPv4Address mdnsMulticastAddressIpv4{ 224, 0, 0, 251 };
        const IPv6Address mdnsMulticastAddressIpv6{ 0xff02, 0, 0, 0, 0, 0, 0, 0xfb };

        class DnsBitmap
        {
        public:
            explicit DnsBitmap(uint8_t index)
                : bitmap(infra::Bit<uint32_t>(index))
            {}

            friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const DnsBitmap& bitmap)
            {
                stream << infra::data << BitReverse(static_cast<uint8_t>(bitmap.bitmap)) << BitReverse(static_cast<uint8_t>(bitmap.bitmap >> 8))
                       << BitReverse(static_cast<uint8_t>(bitmap.bitmap >> 16)) << BitReverse(static_cast<uint8_t>(bitmap.bitmap >> 24));
                return stream;
            }

            friend infra::TextOutputStream& operator<<(infra::TextOutputStream&& stream, const DnsBitmap& bitmap)
            {
                return stream << bitmap;
            }

        private:
            static uint8_t BitReverse(uint8_t v)
            {
                uint8_t result(0);

                for (int i = 0; i != 8; ++i)
                {
                    result <<= 1;
                    if ((v & 1) == 1)
                        result |= 1;
                    v >>= 1;
                }

                return result;
            }

        private:
            uint32_t bitmap;
        };
    }

    BonjourServer::BonjourServer(DatagramFactory& factory, Multicast& multicast, infra::BoundedConstString instance, infra::BoundedConstString serviceName, infra::BoundedConstString type,
        std::optional<IPv4Address> ipv4Address, std::optional<IPv6Address> ipv6Address, uint16_t port, const DnsHostnameParts& text)
        : datagramExchangeIpv4(ipv4Address != std::nullopt ? factory.Listen(*this, mdnsPort, IPVersions::ipv4) : nullptr)
        , datagramExchangeIpv6(ipv6Address != std::nullopt ? factory.Listen(*this, mdnsPort, IPVersions::ipv6) : nullptr)
        , multicast(multicast)
        , instance(instance)
        , serviceName(serviceName)
        , type(type)
        , ipv4Address(ipv4Address)
        , ipv6Address(ipv6Address)
        , port(port)
        , text(text)
    {
        if (ipv4Address != std::nullopt)
            multicast.JoinMulticastGroup(datagramExchangeIpv4, mdnsMulticastAddressIpv4);
        if (ipv6Address != std::nullopt)
            multicast.JoinMulticastGroup(datagramExchangeIpv6, mdnsMulticastAddressIpv6);

        if (datagramExchangeIpv4 != nullptr)
            state.emplace<StateAnnounceIPv4>(*this);
        else
            state.emplace<StateAnnounceIPv6>(*this);
    }

    BonjourServer::~BonjourServer()
    {
        if (ipv4Address != std::nullopt)
            multicast.LeaveMulticastGroup(datagramExchangeIpv4, mdnsMulticastAddressIpv4);
        if (ipv6Address != std::nullopt)
            multicast.LeaveMulticastGroup(datagramExchangeIpv6, mdnsMulticastAddressIpv6);
    }

    void BonjourServer::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from)
    {
        state->DataReceived(std::move(reader), from);
    }

    void BonjourServer::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        state->SendStreamAvailable(std::move(writer));
    }

    BonjourServer::Answer::Answer(BonjourServer& server, uint16_t queryId, infra::StreamWriter& writer, uint16_t answersCount, uint16_t additionalRecordsCount)
        : server(server)
        , writer(writer)
        , answersCount(answersCount)
        , additionalRecordsCount(additionalRecordsCount)
    {
        DnsRecordHeader header{ queryId, DnsRecordHeader::flagsResponse, 0, answersCount, 0, additionalRecordsCount };
        stream << header;
    }

    uint16_t BonjourServer::Answer::Answers() const
    {
        return answersCount;
    }

    uint16_t BonjourServer::Answer::AdditionalRecords() const
    {
        return additionalRecordsCount;
    }

    void BonjourServer::Answer::AddAAnswer()
    {
        ++answersCount;
        if (server.ipv4Address != std::nullopt)
            AddA(DnsHostnameInParts(server.instance)("local"));
        else
            AddNoA(DnsHostnameInParts(server.instance)("local"));
    }

    void BonjourServer::Answer::AddAaaaAnswer()
    {
        ++answersCount;
        if (server.ipv6Address != std::nullopt)
            AddAaaa(DnsHostnameInParts(server.instance)("local"));
        else
            AddNoAaaa(DnsHostnameInParts(server.instance)("local"));
    }

    void BonjourServer::Answer::AddPtrAnswer()
    {
        ++answersCount;
        AddPtr(DnsHostnameInParts(server.serviceName)(server.type)("local"));
    }

    void BonjourServer::Answer::AddSrvAnswer()
    {
        ++answersCount;
        AddSrv(DnsHostnameInParts(server.instance)(server.serviceName)(server.type)("local"));
    }

    void BonjourServer::Answer::AddTxtAnswer()
    {
        ++answersCount;
        AddTxt(DnsHostnameInParts(server.instance)(server.serviceName)(server.type)("local"));
    }

    void BonjourServer::Answer::AddAAdditional()
    {
        ++additionalRecordsCount;
        if (server.ipv4Address != std::nullopt)
            AddA(DnsHostnameInParts(server.instance)("local"));
        else
            AddNoA(DnsHostnameInParts(server.instance)("local"));
    }

    void BonjourServer::Answer::AddAaaaAdditional()
    {
        ++additionalRecordsCount;
        if (server.ipv6Address != std::nullopt)
            AddAaaa(DnsHostnameInParts(server.instance)("local"));
        else
            AddNoAaaa(DnsHostnameInParts(server.instance)("local"));
    }

    void BonjourServer::Answer::AddSrvAdditional()
    {
        ++additionalRecordsCount;
        AddSrv(DnsHostnameInParts(server.instance)(server.serviceName)(server.type)("local"));
    }

    void BonjourServer::Answer::AddTxtAdditional()
    {
        ++additionalRecordsCount;
        AddTxt(DnsHostnameInParts(server.instance)(server.serviceName)(server.type)("local"));
    }

    void BonjourServer::Answer::AddA(const DnsHostnameParts& dnsHostname)
    {
        DnsRecordPayload payload{ DnsType::dnsTypeA, DnsClass::dnsClassIn, std::chrono::seconds(5), sizeof(IPv4Address) };

        stream << infra::text << dnsHostname;
        stream << payload;
        stream << *server.ipv4Address;
    }

    void BonjourServer::Answer::AddNoA(const DnsHostnameParts& dnsHostname)
    {
        DnsRecordPayload payload{ DnsType::dnsTypeNsec, DnsClass::dnsClassIn, std::chrono::seconds(5), static_cast<uint16_t>(6 + dnsHostname.StreamedSize()) };

        stream << infra::text << dnsHostname;
        stream << payload;
        stream << infra::text << dnsHostname;
        stream << static_cast<uint8_t>(0) << static_cast<uint8_t>(4) << infra::text << DnsBitmap(static_cast<uint8_t>(DnsType::dnsTypeAAAA));
    }

    void BonjourServer::Answer::AddAaaa(const DnsHostnameParts& dnsHostname)
    {
        DnsRecordPayload payload{ DnsType::dnsTypeAAAA, DnsClass::dnsClassIn, std::chrono::seconds(5), sizeof(IPv6Address) };

        stream << infra::text << dnsHostname;
        stream << payload;
        stream << services::ToNetworkOrder(*server.ipv6Address);
    }

    void BonjourServer::Answer::AddNoAaaa(const DnsHostnameParts& dnsHostname)
    {
        DnsRecordPayload payload{ DnsType::dnsTypeNsec, DnsClass::dnsClassIn, std::chrono::seconds(5), static_cast<uint16_t>(6 + dnsHostname.StreamedSize()) };

        stream << infra::text << dnsHostname;
        stream << payload;
        stream << infra::text << dnsHostname;
        stream << static_cast<uint8_t>(0) << static_cast<uint8_t>(4) << infra::text << DnsBitmap(static_cast<uint8_t>(DnsType::dnsTypeA));
    }

    void BonjourServer::Answer::AddPtr(const DnsHostnameParts& dnsHostname)
    {
        auto instance = services::DnsHostnameInParts(server.instance)(server.serviceName)(server.type)("local");
        DnsRecordPayload payload{ DnsType::dnsTypePtr, DnsClass::dnsClassIn, std::chrono::seconds(5), static_cast<uint16_t>(instance.StreamedSize()) };

        stream << infra::text << dnsHostname;
        stream << payload;
        stream << infra::text << instance;
    }

    void BonjourServer::Answer::AddSrv(const DnsHostnameParts& dnsHostname)
    {
        auto instance = services::DnsHostnameInParts(server.instance)("local");
        DnsRecordPayload payload{ DnsType::dnsTypeSrv, DnsClass::dnsClassIn, std::chrono::seconds(5), static_cast<uint16_t>(instance.StreamedSize() + 6) };

        infra::BigEndian<uint16_t> priority(0);
        infra::BigEndian<uint16_t> weight(0);
        infra::BigEndian<uint16_t> port(server.port);

        stream << infra::text << dnsHostname;
        stream << payload << priority << weight << port;
        stream << infra::text << instance;
    }

    void BonjourServer::Answer::AddTxt(const DnsHostnameParts& dnsHostname)
    {
        DnsRecordPayload payload{ DnsType::dnsTypeTxt, DnsClass::dnsClassIn, std::chrono::seconds(5), static_cast<uint16_t>(server.text.StreamedSize() - 1) };

        stream << infra::text << dnsHostname;
        stream << payload;
        stream << infra::text << DnsPartsWithoutTermination(server.text);
    }

    BonjourServer::QuestionParser::QuestionParser(BonjourServer& server, infra::StreamReaderWithRewinding& reader)
        : server(server)
        , reader(reader)
    {
        CreateAnswer(countingWriter);
        reader.Rewind(startMarker);
    }

    BonjourServer::QuestionParser::QuestionParser(BonjourServer& server, infra::StreamReaderWithRewinding& reader, infra::StreamWriter& writer)
        : server(server)
        , reader(reader)
    {
        CreateAnswer(countingWriter); // First fill the answersCount and additionalRecordsCount fields
        reader.Rewind(startMarker);
        CreateAnswer(writer);
        reader.Rewind(startMarker);
    }

    bool BonjourServer::QuestionParser::HasAnswer() const
    {
        return answer != std::nullopt && answer->Answers() != 0;
    }

    void BonjourServer::QuestionParser::RequestSendStream(DatagramExchange& datagramExchange, UdpSocket from, UdpSocket to)
    {
        datagramExchange.RequestSendStream(countingWriter.Processed(), to);
    }

    void BonjourServer::QuestionParser::CreateAnswer(infra::StreamWriter& writer)
    {
        stream >> header;

        if (!IsValidQuestion())
            return;

        answer.emplace(server, header.id, writer, answersCount, additionalRecordsCount);

        auto startOfQuestions = reader.ConstructSaveMarker();

        for (auto i = 0; valid && i != header.questionsCount; ++i)
            ReadQuestion();

        reader.Rewind(startOfQuestions);

        for (auto i = 0; valid && i != header.questionsCount; ++i)
            ReadQuestionForAdditionalRecords();

        answersCount = answer->Answers();
        additionalRecordsCount = answer->AdditionalRecords();

        if (!valid)
            answer = std::nullopt;
    }

    bool BonjourServer::QuestionParser::IsValidQuestion() const
    {
        if (stream.Failed())
            return false;

        if ((header.flags & DnsRecordHeader::flagsOpcodeMask) != DnsRecordHeader::flagsOpcodeQuery)
            return false;

        if ((header.flags & DnsRecordHeader::flagsResponse) != DnsRecordHeader::flagsQuery)
            return false;

        return true;
    }

    void BonjourServer::QuestionParser::ReadQuestion()
    {
        ReadHostname();
        stream >> footer;

        if (stream.Failed() || !IsFooterValid())
            valid = false;
        else if (IsQueryForMe())
        {
            switch (static_cast<DnsType>(static_cast<uint16_t>(footer.type)))
            {
                case DnsType::dnsTypeA:
                    answer->AddAAnswer();
                    break;
                case DnsType::dnsTypeAAAA:
                    answer->AddAaaaAnswer();
                    break;
                case DnsType::dnsTypePtr:
                    answer->AddPtrAnswer();
                    break;
                case DnsType::dnsTypeSrv:
                    answer->AddSrvAnswer();
                    break;
                case DnsType::dnsTypeTxt:
                    answer->AddTxtAnswer();
                    break;
                default:
                    std::abort();
            }
        }
    }

    void BonjourServer::QuestionParser::ReadQuestionForAdditionalRecords()
    {
        ReadHostname();
        stream >> footer;

        if (IsQueryForMe())
        {
            switch (static_cast<DnsType>(static_cast<uint16_t>(footer.type)))
            {
                case DnsType::dnsTypePtr:
                    answer->AddTxtAdditional();
                    answer->AddSrvAdditional();
                    answer->AddAAdditional();
                    answer->AddAaaaAdditional();
                    break;
                case DnsType::dnsTypeSrv:
                    answer->AddAAdditional();
                    answer->AddAaaaAdditional();
                    break;
                default:
                    break;
            }
        }
    }

    bool BonjourServer::QuestionParser::IsFooterValid() const
    {
        if (footer.type != infra::enum_cast(DnsType::dnsTypeA) && footer.type != infra::enum_cast(DnsType::dnsTypeAAAA) && footer.type != infra::enum_cast(DnsType::dnsTypePtr) && footer.type != infra::enum_cast(DnsType::dnsTypeSrv) && footer.type != infra::enum_cast(DnsType::dnsTypeTxt))
            return false;

        if (footer.class_ != infra::enum_cast(DnsClass::dnsClassIn))
            return false;

        return true;
    }

    bool BonjourServer::QuestionParser::IsQueryForMe() const
    {
        if (footer.type == infra::enum_cast(DnsType::dnsTypeA) && MyShortInstanceName())
            return true;

        if (footer.type == infra::enum_cast(DnsType::dnsTypeAAAA) && MyShortInstanceName())
            return true;

        if (footer.type == infra::enum_cast(DnsType::dnsTypeSrv) && MyFullInstanceName())
            return true;

        if (footer.type == infra::enum_cast(DnsType::dnsTypeTxt) && MyFullInstanceName())
            return true;

        if (footer.type == infra::enum_cast(DnsType::dnsTypePtr) && MyServiceName())
            return true;

        return false;
    }

    bool BonjourServer::QuestionParser::MyFullInstanceName() const
    {
        std::array<infra::BoundedConstString, 4> service = { { server.instance, server.serviceName, server.type, "local" } };

        if (HostNameIs(service))
            return true;

        return false;
    }

    bool BonjourServer::QuestionParser::MyShortInstanceName() const
    {
        std::array<infra::BoundedConstString, 2> service = { { server.instance, "local" } };

        if (HostNameIs(service))
            return true;

        return false;
    }

    bool BonjourServer::QuestionParser::MyServiceName() const
    {
        std::array<infra::BoundedConstString, 3> service = { { server.serviceName, server.type, "local" } };

        if (HostNameIs(service))
            return true;

        return false;
    }

    void BonjourServer::QuestionParser::ReadHostname()
    {
        auto hostnameStart = reader.ConstructSaveMarker();
        DnsHostnamePartsStream hostnameParts(reader, hostnameStart);

        reconstructedHostname.clear();

        while (!hostnameParts.Empty())
        {
            reconstructedHostname += hostnameParts.Current();
            hostnameParts.ConsumeCurrent();

            if (!hostnameParts.Empty())
                reconstructedHostname += '.';
        }

        hostnameParts.ConsumeStream();
    }

    bool BonjourServer::QuestionParser::HostNameIs(infra::MemoryRange<infra::BoundedConstString> components) const
    {
        DnsHostnamePartsString parts(reconstructedHostname);

        while (!components.empty() && !parts.Empty() && parts.Current() == components.front())
        {
            parts.ConsumeCurrent();
            components.pop_front();
        }

        return parts.Empty() && components.empty();
    }

    BonjourServer::StateIdle::StateIdle(BonjourServer& server)
        : server(server)
    {}

    void BonjourServer::StateIdle::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from)
    {
        if (GetPort(from) != mdnsPort)
            return;

        if (waitingReader != nullptr)
            return;

        waitingReader = std::move(reader);
        QuestionParser question(server, *waitingReader);
        if (question.HasAnswer())
        {
            if (GetAddress(from).Is<services::IPv4Address>())
                question.RequestSendStream(*server.datagramExchangeIpv4, from, MakeUdpSocket(mdnsMulticastAddressIpv4, mdnsPort));
            else
                question.RequestSendStream(*server.datagramExchangeIpv6, from, MakeUdpSocket(mdnsMulticastAddressIpv6, mdnsPort));
        }
        else
            waitingReader = nullptr;
    }

    void BonjourServer::StateIdle::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        assert(waitingReader != nullptr);

        QuestionParser question(server, *waitingReader, *writer);

        waitingReader = nullptr;
    }

    BonjourServer::StateAnnounce::StateAnnounce(BonjourServer& server)
        : server(server)
    {}

    void BonjourServer::StateAnnounce::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from)
    {}

    void BonjourServer::StateAnnounce::WriteAnnounceQuery(infra::StreamWriter& writer)
    {
        Answer answer(server, 0, writer, 5, 0);
        answer.AddPtrAnswer();
        answer.AddSrvAnswer();
        answer.AddTxtAnswer();
        answer.AddAAnswer();
        answer.AddAaaaAnswer();
    }

    BonjourServer::StateAnnounceIPv4::StateAnnounceIPv4(BonjourServer& server)
        : StateAnnounce(server)
    {
        infra::CountingStreamWriter countingWriter;
        WriteAnnounceQuery(countingWriter);

        server.datagramExchangeIpv4->RequestSendStream(countingWriter.Processed(), MakeUdpSocket(mdnsMulticastAddressIpv4, mdnsPort));
    }

    void BonjourServer::StateAnnounceIPv4::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        WriteAnnounceQuery(*writer);
        writer = nullptr;

        if (server.datagramExchangeIpv6 != nullptr)
            server.state.emplace<StateAnnounceIPv6>(server);
        else
            server.state.emplace<StateIdle>(server);
    }

    BonjourServer::StateAnnounceIPv6::StateAnnounceIPv6(BonjourServer& server)
        : StateAnnounce(server)
    {
        infra::CountingStreamWriter countingWriter;
        WriteAnnounceQuery(countingWriter);

        server.datagramExchangeIpv6->RequestSendStream(countingWriter.Processed(), MakeUdpSocket(mdnsMulticastAddressIpv6, mdnsPort));
    }

    void BonjourServer::StateAnnounceIPv6::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        WriteAnnounceQuery(*writer);
        writer = nullptr;

        server.state.emplace<StateIdle>(server);
    }
}
