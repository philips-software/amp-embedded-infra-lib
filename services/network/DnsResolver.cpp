#include "infra/event/EventDispatcher.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "services/network/DnsResolver.hpp"
#include <cctype>

namespace services
{
    const infra::Duration DnsResolver::responseTimeout = std::chrono::seconds(5);

    DnsResolver::DnsResolver(DatagramFactory& datagramFactory, const DnsServers& nameServers, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : datagramFactory(datagramFactory)
        , randomDataGenerator(randomDataGenerator)
        , nameServers(nameServers.nameServers)
    {
        assert(!this->nameServers.empty());
    }

    void DnsResolver::Lookup(NameResolverResult& result)
    {
        assert(!waiting.has_element(result));
        waiting.push_back(result);
        TryResolveNext();
    }

    void DnsResolver::CancelLookup(NameResolverResult& result)
    {
        if (!activeLookup->IsResolving(result))
        {
            assert(waiting.has_element(result));
            waiting.erase(result);
        }
        else
            NameLookupCancelled();
    }

    void DnsResolver::TryResolveNext()
    {
        if (activeLookup == infra::none && !waiting.empty())
        {
            Resolve(waiting.front());
            waiting.pop_front();
        }
    }

    void DnsResolver::Resolve(NameResolverResult& nameLookup)
    {
        activeLookup.Emplace(*this, nameLookup);
        ++currentNameServer;
        if (currentNameServer == nameServers.size())
            currentNameServer = 0;
    }

    void DnsResolver::NameLookupSuccess(NameResolverResult& nameLookup, IPAddress address, infra::TimePoint validUntil)
    {
        NameLookupDone([&nameLookup, &address, &validUntil]() { nameLookup.NameLookupDone(address, validUntil); });
    }

    void DnsResolver::NameLookupFailed(NameResolverResult& nameLookup)
    {
        NameLookupDone([&nameLookup]() { nameLookup.NameLookupFailed(); });
    }

    void DnsResolver::NameLookupCancelled()
    {
        NameLookupDone([]() {});
    }

    void DnsResolver::NameLookupDone(const infra::Function<void(), 3 * sizeof(void*)>& observerCallback)
    {
        activeLookup = infra::none;
        observerCallback();
        TryResolveNext();
    }

    bool DnsResolver::ResourceInner::IsCName() const
    {
        return class_ == QuestionFooter::classIn && type == QuestionFooter::typeCName;
    }

    bool DnsResolver::ResourceInner::IsIPv4Answer() const
    {
        return class_ == QuestionFooter::classIn && type == QuestionFooter::typeA && resourceDataLength == static_cast<uint16_t>(sizeof(IPv4Address));
    }

    bool DnsResolver::ResourceInner::IsNameServer() const
    {
        return class_ == QuestionFooter::classIn && type == QuestionFooter::typeNameServer;
    }

    DnsResolver::ReplyParser::ReplyParser(infra::StreamReaderWithRewinding& reader, infra::BoundedString& hostname)
        : reader(reader)
        , hostname(hostname)
    {
        stream >> header;
        hostnameMatches = ReadAndMatchHostname();
        stream >> footer;
    }

    bool DnsResolver::ReplyParser::AnswerIsForCurrentQuery(UdpSocket from, const IPAddress& currentNameServer, uint16_t queryId) const
    {
        // RFC 5452: Only accept responses from DNS server to which the query is sent
        if (GetAddress(from) != currentNameServer)
            return false;
        if ((header.flags & header.flagsResponse) == 0)
            return false;
        if (header.questionsCount != static_cast<uint16_t>(1))
            return false;
        if (header.id != queryId)
            return false;
        if (!hostnameMatches)
            return false;
        if (footer.type != footer.typeA)
            return false;
        if (footer.class_ != footer.classIn)
            return false;

        return true;
    }

    bool DnsResolver::ReplyParser::Error() const
    {
        return (header.flags & header.flagsErrorMask) != 0;
    }

    bool DnsResolver::ReplyParser::Recurse() const
    {
        return recurse;
    }

    infra::Optional<std::pair<IPAddress, infra::TimePoint>> DnsResolver::ReplyParser::ReadAnswerRecords()
    {
        for (uint16_t answerIndex = 0; answerIndex != header.answersCount; ++answerIndex)
        {
            auto answer = ReadAnswer();

            if (answer.Is<Answer>())
                return infra::MakeOptional(std::make_pair(answer.Get<Answer>().address, answer.Get<Answer>().validUntil));
            else if (answer.Is<CName>())
                recurse = true;
        }

        return infra::none;
    }

    void DnsResolver::ReplyParser::ReadNameServers(infra::BoundedVector<IPAddress>& recursiveDnsServers)
    {
        auto nameServerPosition = reader.ConstructSaveMarker();

        DiscardNameServerRecords();
        ReadAdditionalRecords(recursiveDnsServers, nameServerPosition);
    }

    void DnsResolver::ReplyParser::DiscardNameServerRecords()
    {
        for (uint16_t nameServerIndex = 0; nameServerIndex != header.nameServersCount; ++nameServerIndex)
            DiscardAnswer();
    }

    void DnsResolver::ReplyParser::ReadAdditionalRecords(infra::BoundedVector<IPAddress>& recursiveDnsServers, uint32_t nameServerPosition)
    {
        for (uint16_t additionalIndex = 0; additionalIndex != header.additionalRecordsCount; ++additionalIndex)
        {
            auto answer = ReadNameServer(nameServerPosition, header.nameServersCount);
            if (answer != infra::none && !recursiveDnsServers.full())
            {
                recursiveDnsServers.push_back(*answer);
                recurse = true;
            }
        }
    }

    infra::Variant<DnsResolver::Answer, DnsResolver::CName, DnsResolver::NoAnswer> DnsResolver::ReplyParser::ReadAnswer()
    {
        bool hostnameMatches = ReadAndMatchHostname();
        auto resourceInner = stream.Extract<ResourceInner>();

        if (hostnameMatches)
        {
            if (resourceInner.IsCName())
            {
                hostname.clear();
                ReadCName(resourceInner.resourceDataLength);

                if (!stream.Failed())
                    return CName{};
                else
                    return NoAnswer{};
            }
            else if (resourceInner.IsIPv4Answer())
            {
                auto address = stream.Extract<IPv4Address>();

                if (!stream.Failed())
                    return Answer{ address, infra::Now() + std::chrono::seconds((resourceInner.ttl1 << 16) + resourceInner.ttl2) };
                return NoAnswer{};
            }
        }

        stream.Consume(resourceInner.resourceDataLength);
        return NoAnswer{};
    }

    void DnsResolver::ReplyParser::DiscardAnswer()
    {
        ReadAndMatchHostname();
        auto resourceInner = stream.Extract<ResourceInner>();
        stream.Consume(resourceInner.resourceDataLength);
    }

    infra::Optional<IPAddress> DnsResolver::ReplyParser::ReadNameServer(std::size_t nameServerPosition, std::size_t numNameServers)
    {
        bool nameServerMatches = ReadAndMatchNameServer(nameServerPosition, numNameServers);
        auto resourceInner = stream.Extract<ResourceInner>();

        if (nameServerMatches && resourceInner.IsIPv4Answer())
        {
            auto address = stream.Extract<IPv4Address>();

            if (!stream.Failed())
                return infra::MakeOptional(IPAddress(address));
            else
                return infra::none;
        }

        stream.Consume(resourceInner.resourceDataLength);
        return infra::none;
    }

    bool DnsResolver::ReplyParser::ReadAndMatchHostname()
    {
        HostnamePartsString hostnameParts(hostname);
        return ReadAndMatchHostnameParts(hostnameParts);
    }

    bool DnsResolver::ReplyParser::ReadAndMatchHostnameParts(HostnameParts& hostnameParts)
    {
        auto size = stream.Extract<uint8_t>();

        if ((size & 0xc0) == 0xc0)
            return ReadAndMatchReferenceHostname(size & 0x3f, hostnameParts);
        else
            return ReadAndMatchHostnameWithoutReference(size, hostnameParts);
    }

    bool DnsResolver::ReplyParser::ReadAndMatchReferenceHostname(uint8_t offsetHigh, HostnameParts& hostnameParts)
    {
        auto offsetLow = stream.Extract<uint8_t>();
        uint16_t offset = offsetLow + (offsetHigh << 8);

        auto currentPosition = reader.ConstructSaveMarker();
        if (currentPosition <= offset)
            return false;

        reader.Rewind(offset);

        auto size = stream.Extract<uint8_t>();
        bool result = ReadAndMatchHostnameWithoutReference(size, hostnameParts);

        reader.Rewind(currentPosition);
        return result;
    }

    bool DnsResolver::ReplyParser::ReadAndMatchHostnameWithoutReference(uint8_t size, HostnameParts& hostnameParts)
    {
        bool result = true;
        while (size != 0)
        {
            if (!ReadAndMatchHostnamePart(size, hostnameParts))
                result = false;

            if (result)
                hostnameParts.ConsumeCurrent();

            size = stream.Extract<uint8_t>();
            if ((size & 0xc0) == 0xc0)
                return ReadAndMatchReferenceHostname(size & 0x3f, hostnameParts) && result;
        }

        stream.Consume(size);
        return result && size == 0 && hostnameParts.Empty() && !stream.Failed();
    }

    bool DnsResolver::ReplyParser::ReadAndMatchHostnamePart(uint8_t size, const HostnameParts& hostnameParts)
    {
        if (hostnameParts.Empty() || hostnameParts.Current().size() < size)
        {
            stream.Consume(size);
            return false;
        }

        for (uint8_t i = 0; i != size; ++i)
        {
            auto c = stream.Extract<char>();
            if (std::tolower(hostnameParts.Current()[i]) != std::tolower(c))
            {
                stream.Consume(size - i - 1);
                return false;
            }
        }

        return true;
    }

    void DnsResolver::ReplyParser::ReadCName(uint16_t resourceSize)
    {
        uint8_t partSize = 0;

        for (; resourceSize != 0 && !hostname.full(); --resourceSize)
        {
            if (partSize == 0)
            {
                partSize = stream.Extract<uint8_t>();

                if (partSize == 0)
                {
                    stream.Consume(resourceSize - 1);
                    if (!hostname.empty())
                        hostname.pop_back();
                    return;
                }

                if ((partSize & 0xc0) == 0xc0)
                {
                    ReadCNameReference(partSize & 0x3f);
                    return;
                }
            }
            else
            {
                auto c = stream.Extract<uint8_t>();
                hostname.push_back(c);
                --partSize;

                if (partSize == 0)
                    hostname.push_back('.');
            }
        }
    }

    void DnsResolver::ReplyParser::ReadCNameReference(uint8_t offsetHigh)
    {
        auto offsetLow = stream.Extract<uint8_t>();
        uint16_t offset = offsetLow + (offsetHigh << 8);

        auto currentPosition = reader.ConstructSaveMarker();
        if (currentPosition > offset)
        {
            reader.Rewind(offset);

            ReadCName(static_cast<uint16_t>(currentPosition) - offset);

            reader.Rewind(currentPosition);
        }
    }

    bool DnsResolver::ReplyParser::ReadAndMatchNameServer(std::size_t nameServerPosition, std::size_t numNameServers)
    {
        auto additionalRecordNameStart = reader.ConstructSaveMarker();
        reader.Rewind(nameServerPosition);

        std::size_t i = 0;
        for (; i != numNameServers; ++i)
            if (IsNameServerForAdditionalRecord(additionalRecordNameStart))
                break;

        reader.Rewind(additionalRecordNameStart);
        ReadAndMatchHostname();
        return i != numNameServers;
    }

    bool DnsResolver::ReplyParser::IsNameServerForAdditionalRecord(std::size_t additionalRecordNameStart)
    {
        ReadAndMatchHostname();
        auto resourceInner = stream.Extract<ResourceInner>();

        if (resourceInner.IsNameServer())
        {
            HostnamePartsStream hostnameParts(reader, additionalRecordNameStart);

            if (ReadAndMatchHostnameParts(hostnameParts))
                return true;
        }
        else
            stream.Consume(resourceInner.resourceDataLength);

        return false;
    }

    DnsResolver::ActiveLookup::ActiveLookup(DnsResolver& resolver, NameResolverResult& resolving)
        : resolver(resolver)
        , datagramExchange(resolver.datagramFactory.Listen(*this))
        , queryId(resolver.randomDataGenerator.GenerateRandomData<uint16_t>())
        , resolving(resolving)
        , hostname(resolving.Hostname())
        , nameServers(resolver.nameServers.begin(), resolver.nameServers.end())
        , currentNameServer(nameServers.begin() + resolver.currentNameServer)
    {
        ResolveNextAttempt();
    }

    bool DnsResolver::ActiveLookup::IsResolving(NameResolverResult& resolving) const
    {
        return &resolving == &this->resolving;
    }

    void DnsResolver::ActiveLookup::DataReceived(infra::StreamReaderWithRewinding& reader, UdpSocket from)
    {
        ReplyParser replyParser(reader, hostname);

        if (replyParser.AnswerIsForCurrentQuery(from, GetAddress(DnsUpdSocket()), queryId))
        {
            if (replyParser.Error())
                ResolveNextAttempt();
            else
                TryFindAnswer(replyParser);
        }
    }

    void DnsResolver::ActiveLookup::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);

        QueryHeader header{ queryId, QueryHeader::flagsRecursionDesired, 1, 0, 0, 0 };

        stream << header;
        WriteHostname(stream);
        QuestionFooter footer{ QuestionFooter::typeA, QuestionFooter::classIn };
        stream << footer;

        timeoutTimer.Start(responseTimeout, [this]() { ResolveNextAttempt(); });
    }

    void DnsResolver::ActiveLookup::ResolveNextAttempt()
    {
        if (resolveAttempts == maxAttempts)
            resolver.NameLookupFailed(resolving);
        else
        {
            ++resolveAttempts;
            recursions = 0;
            SelectNextNameServer();
            ResolveAttempt();
        }
    }

    void DnsResolver::ActiveLookup::ResolveRecursion()
    {
        ++recursions;

        if (recursions == maxRecursions)
            resolver.NameLookupFailed(resolving);
        else
            ResolveAttempt();
    }

    void DnsResolver::ActiveLookup::ResolveAttempt()
    {
        datagramExchange->RequestSendStream(QuerySize(), DnsUpdSocket());
    }

    void DnsResolver::ActiveLookup::SelectNextNameServer()
    {
        ++currentNameServer;
        if (currentNameServer == nameServers.end())
            currentNameServer = nameServers.begin();
    }

    void DnsResolver::ActiveLookup::TryFindAnswer(ReplyParser& replyParser)
    {
        auto answer = replyParser.ReadAnswerRecords();
        if (answer != infra::none)
            resolver.NameLookupSuccess(resolving, answer->first, answer->second);
        else
            TryFindRecursiveNameServer(replyParser);
    }

    void DnsResolver::ActiveLookup::TryFindRecursiveNameServer(ReplyParser& replyParser)
    {
        TryNewNameServers(replyParser);

        if (replyParser.Recurse())
            ResolveRecursion();
        else
            ResolveNextAttempt();
    }

    void DnsResolver::ActiveLookup::TryNewNameServers(ReplyParser& replyParser)
    {
        decltype(nameServers) newRecursiveDnsServers;
        replyParser.ReadNameServers(newRecursiveDnsServers);

        if (!newRecursiveDnsServers.empty())
        {
            nameServers = newRecursiveDnsServers;
            currentNameServer = nameServers.begin();
        }
    }

    UdpSocket DnsResolver::ActiveLookup::DnsUpdSocket() const
    {
        return MakeUdpSocket(*currentNameServer, 53);
    }

    std::size_t DnsResolver::ActiveLookup::QuerySize() const
    {
        infra::BoundedConstString hostnameCopy = hostname;
        std::size_t hostnameSize = 1;

        while (!hostnameCopy.empty())
        {
            auto dot = hostnameCopy.find('.');
            if (dot == infra::BoundedConstString::npos)
                break;

            hostnameSize += dot + 1;
            hostnameCopy = hostnameCopy.substr(dot + 1);
        }

        return sizeof(QueryHeader) + hostnameSize + hostnameCopy.size() + 1 + sizeof(QuestionFooter);
    }

    void DnsResolver::ActiveLookup::WriteHostname(infra::DataOutputStream& stream) const
    {
        infra::BoundedConstString hostnameCopy = hostname;

        while (!hostnameCopy.empty())
        {
            auto dot = hostnameCopy.find('.');
            if (dot == infra::BoundedConstString::npos)
                break;

            stream << static_cast<uint8_t>(dot) << infra::text << hostnameCopy.substr(0, dot);
            hostnameCopy = hostnameCopy.substr(dot + 1);
        }

        stream << static_cast<uint8_t>(hostnameCopy.size()) << infra::text << hostnameCopy << infra::data << static_cast<uint8_t>(0);
    }

    DnsResolver::HostnamePartsString::HostnamePartsString(infra::BoundedConstString hostname)
        : hostnameTokens(hostname, '.')
    {}

    infra::BoundedConstString DnsResolver::HostnamePartsString::Current() const
    {
        return hostnameTokens.Token(currentToken);
    }

    void DnsResolver::HostnamePartsString::ConsumeCurrent()
    {
        ++currentToken;
    }

    bool DnsResolver::HostnamePartsString::Empty() const
    {
        return currentToken == hostnameTokens.Size();
    }

    DnsResolver::HostnamePartsStream::HostnamePartsStream(infra::StreamReaderWithRewinding& reader, uint32_t streamPosition)
        : reader(reader)
        , streamPosition(streamPosition)
        , stream(reader, infra::noFail)
    {
        ReadNext();
    }

    infra::BoundedConstString DnsResolver::HostnamePartsStream::Current() const
    {
        return label;
    }

    void DnsResolver::HostnamePartsStream::ConsumeCurrent()
    {
        ReadNext();
    }

    bool DnsResolver::HostnamePartsStream::Empty() const
    {
        return label.empty();
    }

    void DnsResolver::HostnamePartsStream::ReadNext()
    {
        label.clear();

        auto save = reader.ConstructSaveMarker();
        reader.Rewind(streamPosition);

        if (!stream.Empty())
        {
            auto size = stream.Extract<uint8_t>();

            while ((size & 0xc0) == 0xc0)
            {
                uint8_t offsetHigh = size & 0x3f;
                auto offsetLow = stream.Extract<uint8_t>();
                uint16_t offset = offsetLow + (offsetHigh << 8);

                size = 0;

                auto currentPosition = reader.ConstructSaveMarker();
                if (currentPosition <= offset)
                    break;

                reader.Rewind(offset);

                size = stream.Extract<uint8_t>();
            }

            if (size <= label.max_size())
            {
                label.resize(size);
                stream >> infra::StringAsByteRange(label);
            }
        }

        streamPosition = reader.ConstructSaveMarker();
        reader.Rewind(save);
    }
}
