#ifndef SERVICES_MDNS_CLIENT_HPP
#define SERVICES_MDNS_CLIENT_HPP

#include "infra/util/IntrusiveList.hpp"
#include "infra/util/Optional.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Dns.hpp"
#include "services/network/Multicast.hpp"

namespace services
{
    void CreateMdnsHostname(infra::BoundedConstString instance, infra::BoundedConstString serviceName, infra::BoundedConstString type, infra::BoundedString& out);

    class MdnsQuery
        : public infra::IntrusiveList<MdnsQuery>::NodeType
    {
    protected:
        MdnsQuery() = default;
        MdnsQuery(const MdnsQuery& other) = delete;
        MdnsQuery& operator=(const MdnsQuery& other) = delete;
        ~MdnsQuery() = default;

    public:
        virtual infra::BoundedConstString DnsHostname() const = 0;
        virtual services::DnsType DnsType() const = 0;
        virtual bool IsWaiting() const = 0;
        virtual void SetWaiting(bool waiting) = 0;
        virtual services::IPVersions IpVersion() const = 0;
        virtual void CheckAnswer(services::IPVersions ipVersion, infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data) = 0;
        virtual void CheckAdditionalRecord(infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data) = 0;
        virtual void EndOfAnswerNotification() = 0;
    };

    class MdnsClient;

    class MdnsQueryImpl
        : public MdnsQuery
    {
    public:
        MdnsQueryImpl(
            MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString instance, infra::BoundedConstString serviceName, infra::BoundedConstString type, infra::Function<void(infra::ConstByteRange data)> queryHit,
            infra::Function<void(infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data)> queryAdditionalRecordHit = [](infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data) {});
        MdnsQueryImpl(
            MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString serviceName, infra::BoundedConstString type, infra::Function<void(infra::ConstByteRange data)> queryHit,
            infra::Function<void(infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data)> queryAdditionalRecordHit = [](infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data) {});
        MdnsQueryImpl(
            MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString instance, infra::Function<void(infra::ConstByteRange data)> queryHit,
            infra::Function<void(infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data)> queryAdditionalRecordHit = [](infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data) {});

        ~MdnsQueryImpl();

        void SetIpVersion(services::IPVersions ipVersion);

        // Implementation of MdnsQuery
        infra::BoundedConstString DnsHostname() const override;
        services::DnsType DnsType() const override;
        bool IsWaiting() const override;
        services::IPVersions IpVersion() const override;
        void SetWaiting(bool waiting) override;
        void CheckAnswer(services::IPVersions ipVersion, infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data) override;
        void CheckAdditionalRecord(infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data) override;
        void EndOfAnswerNotification() override;

        void Ask(services::IPVersions ipVersion = services::IPVersions::ipv4);

    private:
        MdnsClient& mdnsClient;
        services::DnsType dnsType;
        infra::BoundedString::WithStorage<253> dnsHostname;
        infra::Function<void(infra::ConstByteRange data)> queryHit;
        infra::Function<void(infra::BoundedString hostname, DnsRecordPayload payload, infra::ConstByteRange data)> queryAdditionalRecordHit;
        bool processingQuery = false;
        bool waiting = false;
        services::IPVersions ipVersion = services::IPVersions::ipv4;
    };

    class MdnsClient
        : private DatagramExchangeObserver
    {
    public:
        MdnsClient(DatagramFactory& datagramFactory, Multicast& multicast, IPVersions versions = IPVersions::both);
        ~MdnsClient();

        virtual void RegisterQuery(MdnsQuery& query);
        virtual void UnRegisterQuery(MdnsQuery& query);
        virtual void ActiveQuerySingleShot(MdnsQuery& query);

    private:
        // Implementation of DatagramExchangeObserver
        void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from) override;
        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        infra::detail::IntrusiveListIterator<MdnsQuery> FindNextWaitingQuery();
        void TrySendNextQuery();
        void SendQuery(MdnsQuery& query);
        void ActiveQueryDone();
        bool IsActivelyQuerying();
        void CancelActiveQueryIfEqual(MdnsQuery& query);

    private:
        class ActiveMdnsQuery
        {
        public:
            ActiveMdnsQuery(MdnsClient& mdnsClient, DatagramFactory& datagramFactory, Multicast& multicast, MdnsQuery& query);

            bool IsCurrentQuery(MdnsQuery& query);

            void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer);

        private:
            void SendQuery();

        private:
            MdnsClient& mdnsClient;
            MdnsQuery& query;
        };

        class AnswerParser
        {
        public:
            AnswerParser(MdnsClient& client, services::IPVersions ipVersion, infra::StreamReaderWithRewinding& reader);

        private:
            void Parse();
            bool IsValidAnswer() const;
            void ReadRecord(bool isAnswer);
            void ReadHostname();

        private:
            MdnsClient& client;
            services::IPVersions ipVersion;
            infra::StreamReaderWithRewinding& reader;
            infra::DataInputStream::WithErrorPolicy stream{ reader, infra::noFail };
            std::size_t startMarker{ reader.ConstructSaveMarker() };
            DnsRecordHeader header{};
            infra::BoundedString::WithStorage<253> reconstructedHostname;
            DnsRecordPayload payload{};
            bool valid = true;
        };

    private:
        DatagramFactory& datagramFactory;
        Multicast& multicast;
        IPVersions versions;
        infra::SharedPtr<DatagramExchange> datagramExchange;
        infra::Optional<ActiveMdnsQuery> activeMdnsQuery;
        infra::IntrusiveList<MdnsQuery> queries;
        size_t lastWaitingQueryPosition = 0;
    };
}

#endif
