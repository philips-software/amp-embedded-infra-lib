#include "infra/event/EventDispatcher.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "services/network/DnsResolver.hpp"
#include <cctype>

namespace services
{
    const infra::Duration DnsResolver::responseTimeout = std::chrono::seconds(5);

    DnsResolver::DnsResolver(DatagramFactory& datagramFactory, const DnsServers& dnsServers, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : datagramFactory(datagramFactory)
        , randomDataGenerator(randomDataGenerator)
        , dnsServers(dnsServers.dnsServers)
        , currentDnsServer(this->dnsServers.begin())
    {
        assert(!this->dnsServers.empty());
    }

    void DnsResolver::Lookup(NameResolverResult& result)
    {
        assert(!waiting.has_element(result));
        waiting.push_back(result);
        TryResolveNext();
    }

    void DnsResolver::CancelLookup(NameResolverResult& result)
    {
        if (resolving != &result)
        {
            assert(waiting.has_element(result));
            waiting.erase(result);
        }
        else
            NameLookupCancelled();
    }

    void DnsResolver::DataReceived(infra::StreamReaderWithRewinding& reader, UdpSocket from)
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

    void DnsResolver::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer);

        QueryHeader header{ queryId, QueryHeader::flagsRecursionDesired, 1, 0, 0, 0 };

        stream << header;
        WriteHostname(stream);
        QuestionFooter footer{ QuestionFooter::typeA, QuestionFooter::classIn };
        stream << footer;

        timeoutTimer.Start(responseTimeout, [this]() { ResolveNextAttempt(); });
    }

    void DnsResolver::TryFindAnswer(ReplyParser& replyParser)
    {
        auto answer = replyParser.ReadAnswerRecords();
        if (answer != infra::none)
            NameLookupSuccess(*answer);
        else
            TryFindRecursiveNameServer(replyParser);
    }

    void DnsResolver::TryFindRecursiveNameServer(ReplyParser& replyParser)
    {
        decltype(recursiveDnsServers) newRecursiveDnsServers;
        replyParser.ReadNameServers(newRecursiveDnsServers);

        if (replyParser.Recurse())
        {
            recursiveDnsServers = newRecursiveDnsServers;
            currentRecursiveDnsServer = recursiveDnsServers.begin();
            ResolveRecursion();
        }
        else
            ResolveNextAttempt();
    }

    void DnsResolver::TryResolveNext()
    {
        if (resolving == nullptr && !waiting.empty())
        {
            Resolve(waiting.front());
            waiting.pop_front();
        }
    }

    void DnsResolver::Resolve(NameResolverResult& nameLookup)
    {
        datagramExchange = datagramFactory.Listen(*this);
        randomDataGenerator.GenerateRandomData(infra::MakeByteRange(queryId));
        resolving = &waiting.front();
        resolveAttempts = 0;
        recursions = 0;
        ResolveNextAttempt();
    }

    void DnsResolver::ResolveNextAttempt()
    {
        if (resolveAttempts == maxAttempts)
            NameLookupFailed();
        else
        {
            ++resolveAttempts;
            recursions = 0;
            SelectNextDnsServer();
            hostname = resolving->Hostname();
            ResolveAttempt();
        }
    }

    void DnsResolver::ResolveRecursion()
    {
        ++recursions;

        if (recursions == maxRecursions)
            NameLookupFailed();
        else
            ResolveAttempt();
    }

    void DnsResolver::ResolveAttempt()
    {
        datagramExchange->RequestSendStream(QuerySize(), DnsUpdSocket());
    }

    void DnsResolver::SelectNextDnsServer()
    {
        if (!recursiveDnsServers.empty())
        {
            ++currentRecursiveDnsServer;
            if (currentRecursiveDnsServer == recursiveDnsServers.end())
                currentRecursiveDnsServer = recursiveDnsServers.begin();
        }
        else
        {
            ++currentDnsServer;
            if (currentDnsServer == dnsServers.end())
                currentDnsServer = dnsServers.begin();
        }
    }

    void DnsResolver::NameLookupSuccess(IPAddress address)
    {
        NameLookupDone([address](NameResolverResult& observer) { observer.NameLookupDone(address); });
    }

    void DnsResolver::NameLookupFailed()
    {
        NameLookupDone([](NameResolverResult& observer) { observer.NameLookupFailed(); });
    }

    void DnsResolver::NameLookupCancelled()
    {
        NameLookupDone([](NameResolverResult& observer) {});
    }

    void DnsResolver::NameLookupDone(const infra::Function<void(NameResolverResult& observer), sizeof(IPAddress)>& observerCallback)
    {
        auto resolvingCopy = resolving;
        resolving = nullptr;
        datagramExchange = nullptr;
        timeoutTimer.Cancel();
        recursiveDnsServers.clear();
        observerCallback(*resolvingCopy);
        TryResolveNext();
    }

    UdpSocket DnsResolver::DnsUpdSocket() const
    {
        if (!recursiveDnsServers.empty())
            return MakeUdpSocket(*currentRecursiveDnsServer, 53);
        else
            return MakeUdpSocket(*currentDnsServer, 53);
    }

    std::size_t DnsResolver::QuerySize() const
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

    void DnsResolver::WriteHostname(infra::DataOutputStream& stream) const
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

    bool DnsResolver::ReplyParser::AnswerIsForCurrentQuery(UdpSocket from, const IPAddress& currentDnsServer, uint16_t queryId) const
    {
        // RFC 5452: Only accept responses from DNS server to which the query is sent
        if (GetAddress(from) != currentDnsServer)
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

    infra::Optional<IPAddress> DnsResolver::ReplyParser::ReadAnswerRecords()
    {
        for (uint16_t answerIndex = 0; answerIndex != header.answersCount; ++answerIndex)
        {
            auto answer = ReadAnswer();

            if (answer.Is<Answer>())
                return infra::MakeOptional(answer.Get<Answer>().address);
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
        ResourceInner resourceInner{};
        stream >> resourceInner;

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
                IPv4Address address;
                stream >> address;

                if (!stream.Failed())
                    return Answer{ address };
                return NoAnswer{};
            }
        }

        stream.Consume(resourceInner.resourceDataLength);
        return NoAnswer{};
    }

    void DnsResolver::ReplyParser::DiscardAnswer()
    {
        ReadAndMatchHostname();
        ResourceInner resourceInner{};
        stream >> resourceInner;
        stream.Consume(resourceInner.resourceDataLength);
    }

    infra::Optional<IPAddress> DnsResolver::ReplyParser::ReadNameServer(std::size_t nameServerPosition, std::size_t numNameServers)
    {
        bool nameServerMatches = ReadAndMatchNameServer(nameServerPosition, numNameServers);
        ResourceInner resourceInner{};
        stream >> resourceInner;

        if (nameServerMatches && resourceInner.IsIPv4Answer())
        {
            IPv4Address address;
            stream >> address;

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
        uint8_t size(0);
        stream >> size;

        if ((size & 0xc0) == 0xc0)
            return ReadAndMatchReferenceHostname(size & 0x3f, hostnameParts);
        else
            return ReadAndMatchHostnameWithoutReference(size, hostnameParts);
    }

    bool DnsResolver::ReplyParser::ReadAndMatchReferenceHostname(uint8_t offsetHigh, HostnameParts& hostnameParts)
    {
        uint8_t offsetLow = 0;
        stream >> offsetLow;
        uint16_t offset = offsetLow + (offsetHigh << 8);

        auto currentPosition = reader.ConstructSaveMarker();
        if (currentPosition <= offset)
            return false;

        reader.Rewind(offset);

        uint8_t size = 0;
        stream >> size;
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

            size = 0;
            stream >> size;
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
            char c;
            stream >> c;
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
                stream >> partSize;

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
                uint8_t c = ' ';
                stream >> c;
                hostname.push_back(c);
                --partSize;

                if (partSize == 0)
                    hostname.push_back('.');
            }
        }
    }

    void DnsResolver::ReplyParser::ReadCNameReference(uint8_t offsetHigh)
    {
        uint8_t offsetLow = 0;
        stream >> offsetLow;
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
        auto nameStart = reader.ConstructSaveMarker();
        reader.Rewind(nameServerPosition);

        for (std::size_t i = 0; i != numNameServers; ++i)
        {
            ReadAndMatchHostname();
            ResourceInner resourceInner{};
            stream >> resourceInner;

            if (resourceInner.IsNameServer())
            {
                HostnamePartsStream hostnameParts(reader, nameStart);

                if (ReadAndMatchHostnameParts(hostnameParts))
                {
                    reader.Rewind(nameStart);
                    ReadAndMatchHostname();
                    return true;
                }
            }
            else
                stream.Consume(resourceInner.resourceDataLength);
        }

        reader.Rewind(nameStart);
        ReadAndMatchHostname();
        return false;
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
            uint8_t size;
            stream >> infra::MakeByteRange(size);

            while ((size & 0xc0) == 0xc0)
            {
                uint8_t offsetHigh = size & 0x3f;
                uint8_t offsetLow = 0;
                stream >> offsetLow;
                uint16_t offset = offsetLow + (offsetHigh << 8);

                size = 0;

                auto currentPosition = reader.ConstructSaveMarker();
                if (currentPosition <= offset)
                    break;

                reader.Rewind(offset);

                stream >> size;
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
