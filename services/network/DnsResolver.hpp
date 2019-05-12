#ifndef SERVICES_DNS_NAME_LOOKUP_HPP
#define SERVICES_DNS_NAME_LOOKUP_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/Variant.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/NameResolver.hpp"

namespace services
{
    class DnsResolver
        : public NameResolver
        , public DatagramExchangeObserver
    {
    public:
        DnsResolver(DatagramFactory& datagramFactory, infra::MemoryRange<const IPAddress> dnsServers, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        // Implementation of NameResolver
        virtual void Lookup(NameResolverResult& result) override;
        virtual void CancelLookup(NameResolverResult& result) override;

        // Implementation of DatagramExchangeObserver
        virtual void DataReceived(infra::StreamReaderWithRewinding& reader, UdpSocket from) override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        struct QueryHeader
        {
            static const uint16_t flagsQuery = 0;
            static const uint16_t flagsRecursionDesired = 0x0100;
            static const uint16_t flagsResponse = 0x8000;
            static const uint16_t flagsErrorMask = 0x000f;

            infra::BigEndian<uint16_t> id;
            infra::BigEndian<uint16_t> flags;
            infra::BigEndian<uint16_t> questionsCount;
            infra::BigEndian<uint16_t> answersCount;
            infra::BigEndian<uint16_t> nameServersCount;
            infra::BigEndian<uint16_t> additionalRecordsCount;
        };

        struct QuestionFooter
        {
            static const uint16_t typeA = 1;
            static const uint16_t typeCName = 5;
            static const uint16_t classIn = 1;

            infra::BigEndian<uint16_t> type;
            infra::BigEndian<uint16_t> class_;
        };

        struct ResourceInner
        {
            infra::BigEndian<uint16_t> type;
            infra::BigEndian<uint16_t> class_;
            infra::BigEndian<uint16_t> ttl1;
            infra::BigEndian<uint16_t> ttl2;
            infra::BigEndian<uint16_t> resourceDataLength;
        };

        struct Answer
        {
            IPAddress address;
        };

        struct CName
        {};

        struct NoAnswer
        {};

    private:
        void TryResolveNext();
        void Resolve(NameResolverResult& nameLookup);
        void ResolveNextAttempt();
        void ResolveAttempt();
        void SelectNextDnsServer();
        void NameLookupSuccess(IPAddress address);
        void NameLookupFailed();
        void NameLookupCancelled();
        void NameLookupDone(const infra::Function<void(NameResolverResult& observer), sizeof(IPAddress)>& observerCallback);
        UdpSocket DnsUpdSocket() const;
        std::size_t QuerySize() const;
        bool AnswerIsForCurrentQuery(const QueryHeader& header, const QuestionFooter& footer, UdpSocket from, bool hostnameMatches) const;
        infra::Variant<Answer, CName, NoAnswer> ReadAnswer(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader);
        bool IsCName(const ResourceInner& resourceInner) const;
        bool IsIPv4Answer(const ResourceInner& resourceInner) const;
        void ReadCName(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint16_t resourceSize);
        void ReadCNameReference(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint8_t offsetHigh);
        void WriteHostname(infra::DataOutputStream& stream) const;
        bool ReadAndMatchHostname(infra::StreamReaderWithRewinding& reader) const;
        bool ReadAndMatchReferenceHostname(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint8_t offsetHigh, infra::BoundedConstString hostnameCopy) const;
        bool ReadAndMatchHostnameWithoutReference(infra::DataInputStream& stream, infra::StreamReaderWithRewinding& reader, uint8_t size, infra::BoundedConstString hostnameCopy) const;
        bool ReadAndMatchHostnamePart(infra::DataInputStream& stream, uint8_t size, infra::BoundedConstString hostnameCopy) const;

    private:
        DatagramFactory& datagramFactory;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::MemoryRange<const IPAddress> dnsServers;
        const IPAddress* currentDnsServer;
        infra::IntrusiveList<NameResolverResult> waiting;
        infra::SharedPtr<DatagramExchange> datagramExchange;
        NameResolverResult* resolving = nullptr;
        infra::TimerSingleShot timeoutTimer;
        uint8_t resolveAttempts;
        uint16_t queryId;

        infra::BoundedString::WithStorage<253> hostname;

        static const infra::Duration responseTimeout;
        static const uint8_t maxAttempts = 3;
    };
}

#endif
