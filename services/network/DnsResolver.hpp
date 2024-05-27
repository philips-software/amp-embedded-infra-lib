#ifndef SERVICES_DNS_NAME_RESOLVER_HPP
#define SERVICES_DNS_NAME_RESOLVER_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/Variant.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Dns.hpp"
#include "services/network/NameResolver.hpp"

namespace services
{
    class DnsResolver
        : public NameResolver
    {
    public:
        struct DnsServers
        {
            infra::MemoryRange<const IPAddress> nameServers;
        };

        DnsResolver(DatagramFactory& datagramFactory, const DnsServers& nameServers, hal::SynchronousRandomDataGenerator& randomDataGenerator);

        // Implementation of NameResolver
        void Lookup(NameResolverResult& result) override;
        void CancelLookup(NameResolverResult& result) override;

    private:
        static const infra::Duration responseTimeout;
        static const uint8_t maxAttempts = 3;
        static const uint8_t maxRecursions = 5;

    private:
        struct Answer
        {
            IPAddress address;
            infra::TimePoint validUntil;
        };

        struct CName
        {};

        struct NoAnswer
        {};

        class ReplyParser
        {
        public:
            ReplyParser(infra::StreamReaderWithRewinding& reader, infra::BoundedString& hostname);

            bool AnswerIsForCurrentQuery(UdpSocket from, const IPAddress& currentNameServer, uint16_t queryId) const;
            bool Error() const;
            bool Recurse() const;
            std::optional<std::pair<IPAddress, infra::TimePoint>> ReadAnswerRecords();
            void ReadNameServers(infra::BoundedVector<IPAddress>& recursiveDnsServers);

        private:
            void DiscardNameServerRecords();
            void ReadAdditionalRecords(infra::BoundedVector<IPAddress>& recursiveDnsServers, uint32_t nameServerPosition);
            infra::Variant<Answer, CName, NoAnswer> ReadAnswer();
            void DiscardAnswer();
            std::optional<IPAddress> ReadNameServer(std::size_t nameServerPosition, std::size_t numNameServers);
            bool ReadAndMatchHostname();
            bool ReadAndMatchHostnameParts(DnsHostnameParts& hostnameParts);
            bool ReadAndMatchReferenceHostname(uint8_t offsetHigh, DnsHostnameParts& hostnameParts);
            bool ReadAndMatchHostnameWithoutReference(uint8_t size, DnsHostnameParts& hostnameParts);
            bool ReadAndMatchHostnamePart(uint8_t size, const DnsHostnameParts& hostnameParts);
            void ReadCName(uint16_t resourceSize);
            void ReadCNameReference(uint8_t offsetHigh);
            bool ReadAndMatchNameServer(std::size_t nameServerPosition, std::size_t numNameServers);
            bool IsNameServerForAdditionalRecord(std::size_t additionalRecordNameStart);

        private:
            infra::StreamReaderWithRewinding& reader;
            infra::DataInputStream::WithErrorPolicy stream{ reader, infra::noFail };
            infra::BoundedString& hostname;
            bool recurse = false;
            DnsRecordHeader header{};
            DnsQuestionFooter footer{};
            bool hostnameMatches;
        };

        class ActiveLookup
            : private DatagramExchangeObserver
        {
        public:
            ActiveLookup(DnsResolver& resolver, NameResolverResult& resolving);

            bool IsResolving(NameResolverResult& resolving) const;

        private:
            // Implementation of DatagramExchangeObserver
            void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from) override;
            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

            void ResolveNextAttempt();
            void ResolveRecursion();
            void ResolveAttempt();
            void SelectNextNameServer();
            void TryFindAnswer(ReplyParser& replyParser);
            void TryFindRecursiveNameServer(ReplyParser& replyParser);
            void TryNewNameServers(ReplyParser& replyParser);
            UdpSocket DnsUdpSocket() const;
            std::size_t QuerySize() const;

        private:
            DnsResolver& resolver;
            infra::SharedPtr<DatagramExchange> datagramExchange;
            uint16_t queryId;
            NameResolverResult& resolving;
            infra::TimerSingleShot timeoutTimer;
            uint8_t resolveAttempts = 0;
            uint8_t recursions = 0;

            infra::BoundedString::WithStorage<253> hostname;
            infra::BoundedVector<IPAddress>::WithMaxSize<maxAttempts> nameServers;
            const IPAddress* currentNameServer;

            infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
        };

    private:
        void TryResolveNext();
        void Resolve(NameResolverResult& nameLookup);
        void NameLookupSuccess(NameResolverResult& nameLookup, IPAddress address, infra::TimePoint validUntil);
        void NameLookupFailed(NameResolverResult& nameLookup);
        void NameLookupCancelled();
        void NameLookupDone(const infra::Function<void(), 3 * sizeof(void*)>& observerCallback);

    private:
        DatagramFactory& datagramFactory;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::MemoryRange<const IPAddress> nameServers;
        std::size_t currentNameServer = 0;
        infra::IntrusiveList<NameResolverResult> waiting;
        std::optional<ActiveLookup> activeLookup;
    };
}

#endif
