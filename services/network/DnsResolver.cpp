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
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::noFail);

        QueryHeader header{};
        stream >> header;
        bool hostnameMatches = ReadAndMatchHostname(reader);
        QuestionFooter footer{};
        stream >> footer;

        if (!AnswerIsForCurrentQuery(header, footer, from, hostnameMatches))
            return;

        if ((header.flags & header.flagsErrorMask) != 0)
        {
            ResolveNextAttempt();
            return;
        }

        bool recurse = false;
        for (uint16_t answerIndex = 0; answerIndex != header.answersCount; ++answerIndex)
        {
            auto answer = ReadAnswer(stream, reader);

            if (stream.Failed())
                break;

            if (answer.Is<Answer>())
            {
                NameLookupSuccess(answer.Get<Answer>().address);
                return;
            }
            else if (answer.Is<CName>())
                recurse = true;
        }

        recursiveDnsServers.clear();
        currentRecursiveDnsServer = recursiveDnsServers.begin();
        auto nameServerPosition = reader.ConstructSaveMarker();

        for (uint16_t nameServerIndex = 0; nameServerIndex != header.nameServersCount; ++nameServerIndex)
            DiscardAnswer(stream, reader);

        for (uint16_t additionalIndex = 0; additionalIndex != header.additionalRecordsCount; ++additionalIndex)
        {
            auto answer = ReadNameServer(stream, reader, nameServerPosition, header.nameServersCount);

            if (stream.Failed())
                break;

            if (answer != infra::none && !recursiveDnsServers.full())
            {
                recursiveDnsServers.push_back(*answer);
                recurse = true;
            }
        }

        if (recurse)
            ResolveRecursion();
        else
            ResolveNextAttempt();
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

    infra::Variant<DnsResolver::Answer, DnsResolver::CName, DnsResolver::NoAnswer> DnsResolver::ReadAnswer(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader)
    {
        bool hostnameMatches = ReadAndMatchHostname(reader);
        ResourceInner resourceInner{};
        stream >> resourceInner;

        if (hostnameMatches)
        {
            if (IsCName(resourceInner))
            {
                hostname.clear();
                ReadCName(stream, reader, resourceInner.resourceDataLength);
                return CName{};
            }
            else if (IsIPv4Answer(resourceInner))
            {
                IPv4Address address;
                stream >> address;

                return Answer{ address };
            }
        }

        stream.Consume(resourceInner.resourceDataLength);
        return NoAnswer{};
    }

    void DnsResolver::DiscardAnswer(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader) const
    {
        ReadAndMatchHostname(reader);
        ResourceInner resourceInner{};
        stream >> resourceInner;
        stream.Consume(resourceInner.resourceDataLength);
    }

    infra::Optional<IPAddress> DnsResolver::ReadNameServer(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, std::size_t nameServerPosition, std::size_t numNameServers) const
    {
        bool nameServerMatches = ReadAndMatchNameServer(stream, reader, nameServerPosition, numNameServers);
        ResourceInner resourceInner{};
        stream >> resourceInner;

        if (nameServerMatches && IsIPv4Answer(resourceInner))
        {
            IPv4Address address;
            stream >> address;

            return infra::MakeOptional(IPAddress(address));
        }

        stream.Consume(resourceInner.resourceDataLength);
        return infra::none;
    }

    bool DnsResolver::IsCName(const ResourceInner& resourceInner) const
    {
        return resourceInner.class_ == QuestionFooter::classIn && resourceInner.type == QuestionFooter::typeCName;
    }

    bool DnsResolver::IsNameServer(const ResourceInner& resourceInner) const
    {
        return resourceInner.class_ == QuestionFooter::classIn && resourceInner.type == QuestionFooter::typeNameServer;
    }

    bool DnsResolver::IsIPv4Answer(const ResourceInner& resourceInner) const
    {
        return resourceInner.class_ == QuestionFooter::classIn && resourceInner.type == QuestionFooter::typeA && resourceInner.resourceDataLength == static_cast<uint16_t>(sizeof(IPv4Address));
    }

    void DnsResolver::ReadCName(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint16_t resourceSize)
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
                    ReadCNameReference(stream, reader, partSize & 0x3f);
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

    void DnsResolver::ReadCNameReference(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint8_t offsetHigh)
    {
        uint8_t offsetLow = 0;
        stream >> offsetLow;
        uint16_t offset = offsetLow + (offsetHigh << 8);

        auto currentPosition = reader.ConstructSaveMarker();
        if (currentPosition <= offset)
            return;

        reader.Rewind(offset);

        ReadCName(stream, reader, static_cast<uint16_t>(currentPosition) - offset);

        reader.Rewind(currentPosition);
    }

    bool DnsResolver::AnswerIsForCurrentQuery(const QueryHeader& header, const QuestionFooter& footer, UdpSocket from, bool hostnameMatches) const
    {
        // RFC 5452: Only accept responses from DNS server to which the query is sent
        if (GetAddress(from) != *currentDnsServer)
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

    bool DnsResolver::ReadAndMatchHostnameParts(infra::StreamReaderWithRewinding& reader, HostnameParts& hostnameParts) const
    {
        infra::DataInputStream::WithErrorPolicy stream(reader, infra::noFail);

        uint8_t size(0);
        stream >> size;

        if ((size & 0xc0) == 0xc0)
            return ReadAndMatchReferenceHostname(stream, reader, size & 0x3f, hostnameParts);
        else
            return ReadAndMatchHostnameWithoutReference(stream, reader, size, hostnameParts);
    }

    bool DnsResolver::ReadAndMatchHostname(infra::StreamReaderWithRewinding& reader) const
    {
        HostnamePartsString hostnameParts(hostname);
        return ReadAndMatchHostnameParts(reader, hostnameParts);
    }

    bool DnsResolver::ReadAndMatchNameServer(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, std::size_t nameServerPosition, std::size_t numNameServers) const
    {
        auto nameStart = reader.ConstructSaveMarker();
        reader.Rewind(nameServerPosition);

        for (std::size_t i = 0; i != numNameServers; ++i)
        {
            ReadAndMatchHostname(reader);
            ResourceInner resourceInner{};
            stream >> resourceInner;

            if (IsNameServer(resourceInner))
            {
                HostnamePartsStream hostnameParts(reader, nameStart);

                if (ReadAndMatchHostnameParts(reader, hostnameParts))
                {
                    reader.Rewind(nameStart);
                    ReadAndMatchHostname(reader);
                    return true;
                }
            }
            else
                stream.Consume(resourceInner.resourceDataLength);
        }

        reader.Rewind(nameStart);
        ReadAndMatchHostname(reader);
        return false;
    }

    bool DnsResolver::ReadAndMatchReferenceHostname(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint8_t offsetHigh, HostnameParts& hostnameParts) const
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
        bool result = ReadAndMatchHostnameWithoutReference(stream, reader, size, hostnameParts);

        reader.Rewind(currentPosition);
        return result;
    }

    bool DnsResolver::ReadAndMatchHostnameWithoutReference(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint8_t size, HostnameParts& hostnameParts) const
    {
        bool result = true;
        while (size != 0)
        {
            if (!ReadAndMatchHostnamePart(stream, size, hostnameParts))
                result = false;

            if (result)
                hostnameParts.ConsumeCurrent();

            size = 0;
            stream >> size;
            if ((size & 0xc0) == 0xc0)
                return ReadAndMatchReferenceHostname(stream, reader, size & 0x3f, hostnameParts) && result;
        }

        stream.Consume(size);
        return result && size == 0 && hostnameParts.Empty() && !stream.Failed();
    }

    bool DnsResolver::ReadAndMatchHostnamePart(infra::DataInputStream& stream, uint8_t size, const HostnameParts& hostnameParts) const
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
