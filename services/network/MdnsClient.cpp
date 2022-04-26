#include "services/network/MdnsClient.hpp"
#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/StringOutputStream.hpp"

namespace services
{
    namespace
    {
        const uint16_t mdnsPort = 5353;
        const IPv4Address mdnsMulticastAddressIpv4{ 224, 0, 0, 251 };
        const IPv6Address mdnsMulticastAddressIpv6{ 0xff02, 0, 0, 0, 0, 0, 0, 0xfb };
    }

    void CreateMdnsHostname(infra::BoundedConstString instance, infra::BoundedConstString serviceName, infra::BoundedConstString type, infra::BoundedString& out)
    {
        infra::StringOutputStream stream(out);

        auto start = stream.SaveMarker();
        auto AddPartToStream = [&stream, &start](infra::BoundedConstString part) {
            if (!part.empty())
            {
                if (stream.ProcessedBytesSince(start) != 0)
                    stream << ".";
                stream << part;
            }
        };

        AddPartToStream(instance);
        AddPartToStream(serviceName);
        AddPartToStream(type);
        AddPartToStream("local");
    }

    MdnsQueryImpl::MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString instance, infra::BoundedConstString serviceName, infra::BoundedConstString type, infra::Function<void(infra::ConstByteRange data)> queryHit,
        infra::Function<void(infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data)> queryAdditionalRecordHit)
        : mdnsClient(mdnsClient)
        , dnsType(dnsType)
        , queryHit(queryHit)
        , queryAdditionalRecordHit(queryAdditionalRecordHit)
    {
        if (dnsType == services::DnsType::dnsTypeSrv || dnsType == services::DnsType::dnsTypeTxt)
            CreateMdnsHostname(instance, serviceName, type, dnsHostname);
        else
            std::abort();

        mdnsClient.RegisterQuery(*this);
    }

    MdnsQueryImpl::MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString serviceName, infra::BoundedConstString type, infra::Function<void(infra::ConstByteRange data)> queryHit,
        infra::Function<void(infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data)> queryAdditionalRecordHit)
        : mdnsClient(mdnsClient)
        , dnsType(dnsType)
        , queryHit(queryHit)
        , queryAdditionalRecordHit(queryAdditionalRecordHit)
    {
        if (dnsType == services::DnsType::dnsTypePtr)
            CreateMdnsHostname("", serviceName, type, dnsHostname);
        else
            std::abort();

        mdnsClient.RegisterQuery(*this);
    }

    MdnsQueryImpl::MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString instance, infra::Function<void(infra::ConstByteRange data)> queryHit,
        infra::Function<void(infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data)> queryAdditionalRecordHit)
        : mdnsClient(mdnsClient)
        , dnsType(dnsType)
        , queryHit(queryHit)
        , queryAdditionalRecordHit(queryAdditionalRecordHit)
    {
        if (dnsType == services::DnsType::dnsTypeA || dnsType == services::DnsType::dnsTypeAAAA)
            CreateMdnsHostname(instance, "", "", dnsHostname);
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

    services::IPVersions MdnsQueryImpl::IpVersion() const
    {
        return ipVersion;
    }

    void MdnsQueryImpl::SetIpVersion(services::IPVersions ipVersion)
    {
        this->ipVersion = ipVersion;
    }

    void MdnsQueryImpl::Ask(services::IPVersions ipVersion)
    {
        SetIpVersion(ipVersion);
        mdnsClient.ActiveQuerySingleShot(*this);
    }

    void MdnsQueryImpl::CheckAnswer(services::IPVersions ipVersion, infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data)
    {
        if (this->ipVersion == ipVersion && hostname == dnsHostname && payload.type == dnsType && payload.class_ == DnsClass::dnsClassIn)
        {
            processingQuery = true;
            queryHit(data);
        }
    }

    void MdnsQueryImpl::CheckAdditionalRecord(infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data)
    {
        if (processingQuery)
            queryAdditionalRecordHit(hostname, payload, data);
    }

    void MdnsQueryImpl::EndOfAnswerNotification()
    {
        processingQuery = false;
    }

    MdnsClient::MdnsClient(DatagramFactory& datagramFactory, Multicast& multicast)
        : datagramFactory(datagramFactory)
        , multicast(multicast)
        , datagramExchange(datagramFactory.Listen(*this, mdnsPort, IPVersions::both))
    {
        multicast.JoinMulticastGroup(datagramExchange, mdnsMulticastAddressIpv4);
        multicast.JoinMulticastGroup(datagramExchange, mdnsMulticastAddressIpv6);
    }

    MdnsClient::~MdnsClient()
    {
        multicast.LeaveMulticastGroup(datagramExchange, mdnsMulticastAddressIpv4);
        multicast.LeaveMulticastGroup(datagramExchange, mdnsMulticastAddressIpv6);
    }

    void MdnsClient::RegisterQuery(MdnsQuery& query)
    {
        assert(!queries.has_element(query));
        queries.push_back(query);
    }

    void MdnsClient::UnRegisterQuery(MdnsQuery& query)
    {
        assert(queries.has_element(query));
        CancelActiveQueryIfEqual(query);
        queries.erase(query);
    }

    void MdnsClient::ActiveQuerySingleShot(MdnsQuery& query)
    {
        assert(queries.has_element(query));
        query.SetWaiting(true);

        if (!IsActivelyQuerying())
            TrySendNextQuery();
    }

    void MdnsClient::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from)
    {
        if (GetPort(from) != mdnsPort)
            return;

        auto ipVersion = GetVersion(from);

        AnswerParser answer(*this, ipVersion, *reader);
    }

    void MdnsClient::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        if (activeMdnsQuery)
            activeMdnsQuery->SendStreamAvailable(std::move(writer));
        else
            std::abort();
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
        auto query = FindNextWaitingQuery();
        if (query != queries.end())
            SendQuery(*query);
    }

    void MdnsClient::SendQuery(MdnsQuery& query)
    {
        assert(activeMdnsQuery == infra::none);
        activeMdnsQuery.Emplace(*this, datagramFactory, multicast, query);
    }

    void MdnsClient::ActiveQueryDone()
    {
        activeMdnsQuery = infra::none;
        TrySendNextQuery();
    }

    bool MdnsClient::IsActivelyQuerying()
    {
        return activeMdnsQuery != infra::none;
    }

    void MdnsClient::CancelActiveQueryIfEqual(MdnsQuery& query)
    {
        if (!IsActivelyQuerying())
            return;

        if (activeMdnsQuery->IsCurrentQuery(query))
        {
            activeMdnsQuery = infra::none;
            query.SetWaiting(false);
            TrySendNextQuery();
        }
    }

    MdnsClient::ActiveMdnsQuery::ActiveMdnsQuery(MdnsClient& mdnsClient, DatagramFactory& datagramFactory, Multicast& multicast, MdnsQuery& query)
        : mdnsClient(mdnsClient)
        , query(query)
    {
        SendQuery();
    }

    bool MdnsClient::ActiveMdnsQuery::IsCurrentQuery(MdnsQuery& query)
    {
        return &this->query == &query;
    }

    void MdnsClient::ActiveMdnsQuery::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);

        DnsRecordHeader header{ 0, 0, 1, 0, 0, 0 };
        DnsHostnamePartsString dnsHostname(query.DnsHostname());
        DnsQuestionFooter footer{ query.DnsType(), DnsClass::dnsClassIn };

        stream << header;
        stream << infra::text << dnsHostname;
        stream << footer;

        query.SetWaiting(false);

        writer = nullptr;
        mdnsClient.ActiveQueryDone();
    }

    void MdnsClient::ActiveMdnsQuery::SendQuery()
    {
        infra::BoundedConstString hostnameCopy = query.DnsHostname();
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

        switch (query.IpVersion())
        {
            case services::IPVersions::ipv4:
                mdnsClient.datagramExchange->RequestSendStream(querySize, MakeUdpSocket(mdnsMulticastAddressIpv4, mdnsPort));
                break;
            case services::IPVersions::ipv6:
                mdnsClient.datagramExchange->RequestSendStream(querySize, MakeUdpSocket(mdnsMulticastAddressIpv6, mdnsPort));
                break;
            default:
                std::abort();
        }
    }

    MdnsClient::AnswerParser::AnswerParser(MdnsClient& client, services::IPVersions ipVersion, infra::StreamReaderWithRewinding& reader)
        : client(client)
        , reader(reader)
        , ipVersion(ipVersion)
    {
        Parse();
    }

    void MdnsClient::AnswerParser::Parse()
    {
        stream >> header;

        if (!IsValidAnswer())
            return;

        for (auto i = 0; valid && i != header.answersCount; ++i)
            ReadRecord(true);

        for (auto i = 0; valid && i != header.additionalRecordsCount; ++i)
            ReadRecord(false);

        for (auto& query : client.queries)
            query.EndOfAnswerNotification();
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

    void MdnsClient::AnswerParser::ReadRecord(bool isAnswer)
    {
        ReadHostname();
        stream >> payload;

        auto data = stream.ContiguousRange(payload.resourceDataLength);

        if (stream.Failed())
            valid = false;
        else
            for (auto& query : client.queries)
                if (isAnswer)
                    query.CheckAnswer(ipVersion, reconstructedHostname, payload, data);
                else
                    query.CheckAdditionalRecord(reconstructedHostname, payload, data);
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
