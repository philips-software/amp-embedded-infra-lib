#include "infra/event/EventDispatcher.hpp"
#include "services/network/DnsResolver.hpp"
#include <cctype>

namespace services
{
    const infra::Duration DnsResolver::responseTimeout = std::chrono::seconds(5);

    DnsResolver::DnsResolver(DatagramFactory& datagramFactory, infra::MemoryRange<const IPAddress> dnsServers, hal::SynchronousRandomDataGenerator& randomDataGenerator)
        : datagramFactory(datagramFactory)
        , randomDataGenerator(randomDataGenerator)
        , dnsServers(dnsServers)
        , currentDnsServer(dnsServers.begin())
    {
        assert(!dnsServers.empty());
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

        bool obtainedCName = false;
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
                obtainedCName = true;
        }

        if (obtainedCName)
            ResolveAttempt();
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
        ResolveNextAttempt();
    }

    void DnsResolver::ResolveNextAttempt()
    {
        if (resolveAttempts == maxAttempts)
            NameLookupFailed();
        else
        {
            ++resolveAttempts;
            SelectNextDnsServer();
            hostname = resolving->Hostname();
            ResolveAttempt();
        }
    }

    void DnsResolver::ResolveAttempt()
    {
        datagramExchange->RequestSendStream(QuerySize(), DnsUpdSocket());
    }

    void DnsResolver::SelectNextDnsServer()
    {
        ++currentDnsServer;
        if (currentDnsServer == dnsServers.end())
            currentDnsServer = dnsServers.begin();
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

    bool DnsResolver::IsCName(const ResourceInner& resourceInner) const
    {
        return resourceInner.class_ == QuestionFooter::classIn && resourceInner.type == QuestionFooter::typeCName;
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

    bool DnsResolver::ReadAndMatchHostname(infra::StreamReaderWithRewinding& reader) const
    {
        infra::BoundedConstString hostnameCopy = hostname;

        infra::DataInputStream::WithErrorPolicy stream(reader, infra::noFail);

        uint8_t size(0);
        stream >> size;

        if ((size & 0xc0) == 0xc0)
            return ReadAndMatchReferenceHostname(stream, reader, size & 0x3f, hostnameCopy);
        else
            return ReadAndMatchHostnameWithoutReference(stream, reader, size, hostnameCopy);
    }

    bool DnsResolver::ReadAndMatchReferenceHostname(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint8_t offsetHigh, infra::BoundedConstString hostnameCopy) const
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
        bool result = ReadAndMatchHostnameWithoutReference(stream, reader, size, hostnameCopy);

        reader.Rewind(currentPosition);
        return result;
    }

    bool DnsResolver::ReadAndMatchHostnameWithoutReference(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint8_t size, infra::BoundedConstString hostnameCopy) const
    {
        bool result = true;
        while (size != 0 && !hostnameCopy.empty())
        {
            if (!ReadAndMatchHostnamePart(stream, size, hostnameCopy))
                result = false;

            if (!result || hostnameCopy.size() == size)
                hostnameCopy.clear();
            else if (hostnameCopy[size] != '.')
                result = false;
            else
                hostnameCopy = hostnameCopy.substr(size + 1);

            size = 0;
            stream >> size;
            if ((size & 0xc0) == 0xc0)
                return result && ReadAndMatchReferenceHostname(stream, reader, size & 0x3f, hostnameCopy);
        }

        stream.Consume(size);
        return result && size == 0 && hostnameCopy.empty() && !stream.Failed();
    }

    bool DnsResolver::ReadAndMatchHostnamePart(infra::DataInputStream& stream, uint8_t size, infra::BoundedConstString hostnameCopy) const
    {
        if (hostnameCopy.size() < size)
        {
            stream.Consume(size);
            return false;
        }

        for (uint8_t i = 0; i != size; ++i)
        {
            char c;
            stream >> c;
            if (std::tolower(hostnameCopy[i]) != std::tolower(c))
            {
                stream.Consume(size - i - 1);
                return false;
            }
        }

        return true;
    }
}
