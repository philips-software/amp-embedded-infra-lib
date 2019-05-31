#ifndef SERVICES_DNS_NAME_RESOLVER_HPP
#define SERVICES_DNS_NAME_RESOLVER_HPP

#include "hal/synchronous_interfaces/SynchronousRandomDataGenerator.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/Tokenizer.hpp"
#include "infra/util/Variant.hpp"
#include "services/network/Datagram.hpp"
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
        virtual void Lookup(NameResolverResult& result) override;
        virtual void CancelLookup(NameResolverResult& result) override;

    private:
        static const infra::Duration responseTimeout;
        static const uint8_t maxAttempts = 3;
        static const uint8_t maxRecursions = 5;

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
            static const uint16_t typeNameServer = 2;
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

            bool IsCName() const;
            bool IsIPv4Answer() const;
            bool IsNameServer() const;
        };

        struct Answer
        {
            IPAddress address;
        };

        struct CName
        {};

        struct NoAnswer
        {};

        class HostnameParts
        {
        protected:
            HostnameParts() = default;
            HostnameParts(const HostnameParts& other) = delete;
            HostnameParts& operator=(const HostnameParts& other) = delete;
            ~HostnameParts() = default;

        public:
            virtual infra::BoundedConstString Current() const = 0;
            virtual void ConsumeCurrent() = 0;
            virtual bool Empty() const = 0;
        };

        class HostnamePartsString
            : public HostnameParts
        {
        public:
            HostnamePartsString(infra::BoundedConstString hostname);

            virtual infra::BoundedConstString Current() const override;
            virtual void ConsumeCurrent() override;
            virtual bool Empty() const override;

        private:
            infra::Tokenizer hostnameTokens;
            std::size_t currentToken = 0;
        };

        class HostnamePartsStream
            : public HostnameParts
        {
        public:
            HostnamePartsStream(infra::StreamReaderWithRewinding& reader, uint32_t streamPosition);

            virtual infra::BoundedConstString Current() const override;
            virtual void ConsumeCurrent() override;
            virtual bool Empty() const override;

        private:
            void ReadNext();

        private:
            infra::StreamReaderWithRewinding& reader;
            uint32_t streamPosition;
            infra::DataInputStream::WithErrorPolicy stream;
            infra::BoundedString::WithStorage<63> label;
        };

        class ReplyParser
        {
        public:
            ReplyParser(infra::StreamReaderWithRewinding& reader, infra::BoundedString& hostname);

            bool AnswerIsForCurrentQuery(UdpSocket from, const IPAddress& currentNameServer, uint16_t queryId) const;
            bool Error() const;
            bool Recurse() const;
            infra::Optional<IPAddress> ReadAnswerRecords();
            void ReadNameServers(infra::BoundedVector<IPAddress>& recursiveDnsServers);

        private:
            void DiscardNameServerRecords();
            void ReadAdditionalRecords(infra::BoundedVector<IPAddress>& recursiveDnsServers, uint32_t nameServerPosition);
            infra::Variant<Answer, CName, NoAnswer> ReadAnswer();
            void DiscardAnswer();
            infra::Optional<IPAddress> ReadNameServer(std::size_t nameServerPosition, std::size_t numNameServers);
            bool ReadAndMatchHostname();
            bool ReadAndMatchHostnameParts(HostnameParts& hostnameParts);
            bool ReadAndMatchReferenceHostname(uint8_t offsetHigh, HostnameParts& hostnameParts);
            bool ReadAndMatchHostnameWithoutReference(uint8_t size, HostnameParts& hostnameParts);
            bool ReadAndMatchHostnamePart(uint8_t size, const HostnameParts& hostnameParts);
            void ReadCName(uint16_t resourceSize);
            void ReadCNameReference(uint8_t offsetHigh);
            bool ReadAndMatchNameServer(std::size_t nameServerPosition, std::size_t numNameServers);
            bool IsNameServerForAdditionalRecord(std::size_t additionalRecordNameStart);

        private:
            infra::StreamReaderWithRewinding& reader;
            infra::DataInputStream::WithErrorPolicy stream{ reader, infra::noFail };
            infra::BoundedString& hostname;
            bool recurse = false;
            QueryHeader header{};
            QuestionFooter footer{};
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
            virtual void DataReceived(infra::StreamReaderWithRewinding& reader, UdpSocket from) override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

            void ResolveNextAttempt();
            void ResolveRecursion();
            void ResolveAttempt();
            void SelectNextNameServer();
            void TryFindAnswer(ReplyParser& replyParser);
            void TryFindRecursiveNameServer(ReplyParser& replyParser);
            void TryNewNameServers(ReplyParser& replyParser);
            UdpSocket DnsUpdSocket() const;
            std::size_t QuerySize() const;
            void WriteHostname(infra::DataOutputStream& stream) const;

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
        };

    private:
        void TryResolveNext();
        void Resolve(NameResolverResult& nameLookup);
        void NameLookupSuccess(NameResolverResult& nameLookup, IPAddress address);
        void NameLookupFailed(NameResolverResult& nameLookup);
        void NameLookupCancelled();
        void NameLookupDone(const infra::Function<void(), sizeof(IPAddress) + sizeof(void*)>& observerCallback);

    private:
        DatagramFactory& datagramFactory;
        hal::SynchronousRandomDataGenerator& randomDataGenerator;
        infra::MemoryRange<const IPAddress> nameServers;
        std::size_t currentNameServer = 0;
        infra::IntrusiveList<NameResolverResult> waiting;
        infra::Optional<ActiveLookup> activeLookup;
    };
}

#endif
