#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "services/network/MdnsClient.hpp"

namespace services
{
    namespace
    {
        const uint16_t mdnsPort = 5353;
        const IPv4Address mdnsMulticastAddressIpv4{ 224, 0, 0, 251 };
        const IPv6Address mdnsMulticastAddressIpv6{ 0xff02, 0, 0, 0, 0, 0, 0, 0xfb };
    }

    void CreateDnsHostname(infra::BoundedConstString instance, infra::BoundedConstString serviceType, infra::BoundedConstString protocol, infra::BoundedString& out)
    {
        infra::StringOutputStream stream(out);

        auto start = stream.SaveMarker();
        auto AddPartToStream = [&stream, &start](infra::BoundedConstString part)
            {
                if (!part.empty())
                {
                    if (stream.ProcessedBytesSince(start) != 0)
                        stream << ".";
                    stream << part;
                }
            };

        AddPartToStream(instance);
        AddPartToStream(serviceType);
        AddPartToStream(protocol);
        AddPartToStream("local");
    }

    MdnsQueryImpl::MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString instance, infra::BoundedConstString serviceType, infra::BoundedConstString protocol, infra::Function<void(infra::ConstByteRange data)> result)
        : mdnsClient(mdnsClient)
        , dnsType(dnsType)
        , result(result)
    {
        if (dnsType == services::DnsType::dnsTypeSrv || dnsType == services::DnsType::dnsTypeTxt)
            CreateDnsHostname(instance, serviceType, protocol, dnsHostname);
        else
            std::abort();

        mdnsClient.RegisterQuery(*this);
    }

    MdnsQueryImpl::MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString serviceType, infra::BoundedConstString protocol, infra::Function<void(infra::ConstByteRange data)> result)
        : mdnsClient(mdnsClient)
        , dnsType(dnsType)
        , result(result)
    {
        if (dnsType == services::DnsType::dnsTypePtr)
            CreateDnsHostname("", serviceType, protocol, dnsHostname);
        else
            std::abort();
        
        mdnsClient.RegisterQuery(*this);
    }

    MdnsQueryImpl::MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString instance, infra::Function<void(infra::ConstByteRange data)> result)
        : mdnsClient(mdnsClient)
        , dnsType(dnsType)
        , result(result)
    {
        if (dnsType == services::DnsType::dnsTypeA || dnsType == services::DnsType::dnsTypeAAAA)
            CreateDnsHostname(instance, "", "", dnsHostname);
        else
            std::abort();
        
        mdnsClient.RegisterQuery(*this);
    }

    MdnsQueryImpl::~MdnsQueryImpl()
    {
        mdnsClient.UnRegisterQuery(*this);
    }

    infra::BoundedConstString MdnsQueryImpl::DnsHostname() const
    {
        return dnsHostname;
    }

    services::DnsType MdnsQueryImpl::DnsType() const
    {
        return dnsType;
    }

    bool MdnsQueryImpl::IsWaiting() const
    {
        return waiting;
    }

    void MdnsQueryImpl::SetWaiting(bool waiting)
    {
        this->waiting = waiting;
    }

    void MdnsQueryImpl::Ask()
    {
        mdnsClient.ActiveQuerySingleShot(*this);
    }

    void MdnsQueryImpl::CheckQuery(infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data)
    {
        if (hostname == dnsHostname && payload.type == dnsType && payload.class_ == DnsClass::dnsClassIn)
            result(data);
    }

    MdnsClient::MdnsClient(DatagramFactory& datagramFactory, Multicast& multicast)
        : datagramFactory(datagramFactory)
        , multicast(multicast)
    {}

    void MdnsClient::RegisterQuery(MdnsQuery& query)
    {
        assert(!queries.has_element(query));
        queries.push_back(query);

        if (queryHandler == infra::none)
            queryHandler.Emplace(*this);
    }

    void MdnsClient::UnRegisterQuery(MdnsQuery& query)
    {
        assert(queries.has_element(query));
        queryHandler->CancelActiveQueryIfEqual(query);
        queries.erase(query);

        if (queries.empty())
            queryHandler = infra::none;
    }

    void MdnsClient::ActiveQuerySingleShot(MdnsQuery& query)
    {
        assert(queries.has_element(query));
        query.SetWaiting(true);
        TrySendNextQuery();
    }

    infra::detail::IntrusiveListIterator<MdnsQuery> MdnsClient::FindNextWaitingQuery()
    {
        lastWaitingQueryPosition = std::min(lastWaitingQueryPosition, queries.size());
        auto lastWaitingIterator = std::next(queries.begin(), lastWaitingQueryPosition);

        for (auto current = lastWaitingIterator; current != queries.end(); ++current)
            if (current->IsWaiting())
            {
                lastWaitingQueryPosition = std::distance(queries.begin(), current);
                return current;
            }

        for (auto current = queries.begin(); current != lastWaitingIterator; ++current)
            if (current->IsWaiting())
            {
                lastWaitingQueryPosition = std::distance(queries.begin(), current);
                return current;
            }

        lastWaitingQueryPosition = 0;
        return queries.end();
    }

    void MdnsClient::TrySendNextQuery()
    {
        if (!queryHandler->IsActiveQuerying())
        {
            auto query = FindNextWaitingQuery();
            if (query != queries.end())
                queryHandler->SendQuery(*query);
        }
    }

    MdnsClient::QueryHandler::QueryHandler(MdnsClient& client)
        : client(client)
        , datagramExchange(client.datagramFactory.Listen(*this, mdnsPort, IPVersions::ipv4))
    {
        client.multicast.JoinMulticastGroup(datagramExchange, mdnsMulticastAddressIpv4);
    }

    MdnsClient::QueryHandler::~QueryHandler()
    {
        client.multicast.LeaveMulticastGroup(datagramExchange, mdnsMulticastAddressIpv4);
    }

    void MdnsClient::QueryHandler::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from)
    {
        if (GetPort(from) != mdnsPort)
            return;

        AnswerParser answer(client, *reader);
    }

    void MdnsClient::QueryHandler::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        if (IsActiveQuerying() && waitingQuery->IsWaiting())
        {
            infra::DataOutputStream::WithErrorPolicy stream(*writer);

            DnsRecordHeader header{ 0, DnsRecordHeader::flagsRecursionDesired, 1, 0, 0, 0 };
            DnsHostnamePartsString query(waitingQuery->DnsHostname());
            DnsQuestionFooter footer{ waitingQuery->DnsType(), DnsClass::dnsClassIn };

            stream << header;
            stream << infra::text << query;
            stream << footer;

            waitingQuery->SetWaiting(false);
            waitingQuery = nullptr;
        }

        writer = nullptr;
        client.TrySendNextQuery();
    }

    void MdnsClient::QueryHandler::SendQuery(MdnsQuery& query)
    {
        //assert
        waitingQuery = &query;

        infra::BoundedConstString hostnameCopy = waitingQuery->DnsHostname();
        std::size_t hostnameSize = 1;

        while (!hostnameCopy.empty())
        {
            auto dot = hostnameCopy.find('.');
            if (dot == infra::BoundedConstString::npos)
                break;

            hostnameSize += dot + 1;
            hostnameCopy = hostnameCopy.substr(dot + 1);
        }

        std::size_t querySize = sizeof(DnsRecordHeader) + hostnameSize + hostnameCopy.size() + 1 + sizeof(DnsQuestionFooter);

        datagramExchange->RequestSendStream(querySize, MakeUdpSocket(mdnsMulticastAddressIpv4, mdnsPort));
    }

    bool MdnsClient::QueryHandler::IsActiveQuerying()
    {
        return waitingQuery != nullptr;
    }

    void MdnsClient::QueryHandler::CancelActiveQueryIfEqual(MdnsQuery& query)
    {
        if (waitingQuery == &query)
            waitingQuery->SetWaiting(false);
            waitingQuery = nullptr;
    }

    MdnsClient::AnswerParser::AnswerParser(MdnsClient& client, infra::StreamReaderWithRewinding& reader)
        : client(client)
        , reader(reader)
    {
        Parse();
    }

    void MdnsClient::AnswerParser::Parse()
    {
        stream >> header;

        if (!IsValidAnswer())
            return;

        for (auto i = 0; valid && i != header.answersCount; ++i)
            ReadAnswer();

        for (auto i = 0; valid && i != header.additionalRecordsCount; ++i)
            ReadAnswer();
    }

    bool MdnsClient::AnswerParser::IsValidAnswer() const
    {
        if (stream.Failed())
            return false;

        if ((header.flags & DnsRecordHeader::flagsResponse) == DnsRecordHeader::flagsQuery)
            return false;

        if ((header.flags & DnsRecordHeader::flagsOpcodeMask) != DnsRecordHeader::flagsOpcodeQuery)
            return false;

        if ((header.flags & DnsRecordHeader::flagsErrorMask) != DnsRecordHeader::flagsNoError)
            return false;

        if (header.questionsCount != 0)
            return false;

        if (header.answersCount == 0)
            return false;

        if (header.nameServersCount != 0)
            return false;

        return true;
    }

    void MdnsClient::AnswerParser::ReadAnswer()
    {
        ReadHostname();
        stream >> payload;

        auto range = stream.ContiguousRange(payload.resourceDataLength);

        if (stream.Failed())
            valid = false;
        else
            for (auto& query : client.queries)
                query.CheckQuery(reconstructedHostname, payload, range);
    }

    void MdnsClient::AnswerParser::ReadHostname()
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
}
