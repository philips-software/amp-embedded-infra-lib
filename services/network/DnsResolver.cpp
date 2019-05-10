#include "infra/event/EventDispatcher.hpp"
#include "services/network/DnsResolver.hpp"

namespace services
{
    const infra::Duration DnsResolver::responseTimeout = std::chrono::seconds(5);

    DnsResolver::DnsResolver(DatagramFactory& datagramFactory, infra::MemoryRange<const services::IPAddress> dnsServers, hal::SynchronousRandomDataGenerator& randomDataGenerator)
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
        bool hostnameMatches = ReadAndMatchHostname(stream);
        QuestionFooter footer{};
        stream >> footer;

        if (!AnswerIsForCurrentQuery(header, footer, from, hostnameMatches))
            return;

        if ((header.flags & header.flagsErrorMask) != 0)
        {
            ResolveNextAttempt();
            return;
        }

        for (uint16_t answer = 0; answer != header.answersCount; ++answer)
        {
            auto address = ReadAnswer(stream);

            if (stream.Failed())
                break;

            if (address != infra::none)
            {
                NameLookupSuccess(*address);
                return;
            }
        }

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
            datagramExchange->RequestSendStream(QuerySize(), DnsUpdSocket());
        }
    }

    void DnsResolver::SelectNextDnsServer()
    {
        ++currentDnsServer;
        if (currentDnsServer == dnsServers.end())
            currentDnsServer = dnsServers.begin();
    }

    void DnsResolver::NameLookupSuccess(services::IPv4Address address)
    {
        NameLookupDone([address](services::NameResolverResult& observer) { observer.NameLookupDone(address); });
    }

    void DnsResolver::NameLookupFailed()
    {
        NameLookupDone([](services::NameResolverResult& observer) { observer.NameLookupFailed(); });
    }

    void DnsResolver::NameLookupCancelled()
    {
        NameLookupDone([](services::NameResolverResult& observer) {});
    }

    void DnsResolver::NameLookupDone(const infra::Function<void(services::NameResolverResult& observer)>& observerCallback)
    {
        auto resolvingCopy = resolving;
        resolving = nullptr;
        datagramExchange = nullptr;
        timeoutTimer.Cancel();
        observerCallback(*resolvingCopy);
        TryResolveNext();
    }

    services::UdpSocket DnsResolver::DnsUpdSocket() const
    {
        return services::MakeUdpSocket(*currentDnsServer, 53);
    }

    std::size_t DnsResolver::QuerySize() const
    {
        infra::BoundedConstString hostname = resolving->Hostname();

        std::size_t hostnameSize = 1;

        while (!hostname.empty())
        {
            auto dot = hostname.find('.');
            if (dot == infra::BoundedConstString::npos)
                break;

            hostnameSize += dot + 1;
            hostname = hostname.substr(dot + 1);
        }

        return sizeof(QueryHeader) + hostnameSize + hostname.size() + 1 + sizeof(QuestionFooter);
    }

    infra::Optional<services::IPv4Address> DnsResolver::ReadAnswer(infra::DataInputStream& stream) const
    {
        SkipName(stream);
        ResourceInner resourceInner{};
        stream >> resourceInner;

        if (resourceInner.class_ != QuestionFooter::classIn || resourceInner.type != QuestionFooter::typeA)
            stream.Consume(resourceInner.resourceDataLength);
        else if (resourceInner.resourceDataLength == static_cast<uint16_t>(sizeof(services::IPv4Address)))
        {
            services::IPv4Address address;
            stream >> address;

            return infra::MakeOptional(address);
        }

        return infra::none;
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
        infra::BoundedConstString hostname = resolving->Hostname();

        while (!hostname.empty())
        {
            auto dot = hostname.find('.');
            if (dot == infra::BoundedConstString::npos)
                break;
             
            stream << static_cast<uint8_t>(dot) << infra::text << hostname.substr(0, dot);
            hostname = hostname.substr(dot + 1);
        }

        stream << static_cast<uint8_t>(hostname.size()) << infra::text << hostname << infra::data << static_cast<uint8_t>(0);
    }

    bool DnsResolver::ReadAndMatchHostname(infra::DataInputStream& stream) const
    {
        infra::BoundedConstString hostname = resolving->Hostname();

        uint8_t size(0);
        stream >> size;

        while (size != 0 && !hostname.empty())
        {
            for (; size != 0 && !hostname.empty(); --size)
            {
                char c;
                stream >> c;
                if (hostname.front() != c)
                    return false;
                hostname = hostname.substr(1);
            }

            if (hostname.empty())
                break;

            if (hostname.front() != '.')
                break;
            hostname = hostname.substr(1);
            size = 0;
            stream >> size;
        }

        stream >> size;

        return !stream.Failed() && size == 0 && hostname.empty();
    }

    void DnsResolver::SkipName(infra::DataInputStream& stream) const
    {
        uint8_t size(0);
        stream >> size;

        if ((size & 0xc0) == 0xc0)
            stream >> size;
        else
            while (size != 0)
            {
                stream.Consume(size);

                size = 0;
                stream >> size;
            }
    }
}
