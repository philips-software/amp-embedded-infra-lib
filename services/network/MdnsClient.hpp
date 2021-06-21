#ifndef SERVICES_MDNS_CLIENT_HPP
#define SERVICES_MDNS_CLIENT_HPP

#include "infra/util/IntrusiveList.hpp"
#include "infra/util/Optional.hpp"
#include "services/network/Datagram.hpp"
#include "services/network/Dns.hpp"
#include "services/network/Multicast.hpp"

namespace services
{
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
        virtual void CheckQuery(infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data) = 0;
    };

    class MdnsClient;

    class MdnsQueryImpl
        : public MdnsQuery
    {
    public:
        MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString instance, infra::BoundedConstString serviceName, infra::BoundedConstString type, infra::Function<void(infra::ConstByteRange data)> queryHit);
        MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString serviceName, infra::BoundedConstString type, infra::Function<void(infra::ConstByteRange data)> queryHit);
        MdnsQueryImpl(MdnsClient& mdnsClient, services::DnsType dnsType, infra::BoundedConstString instance, infra::Function<void(infra::ConstByteRange data)> queryHit);

        ~MdnsQueryImpl();

        // Implementation of MdnsQuery
        virtual infra::BoundedConstString DnsHostname() const override;
        virtual services::DnsType DnsType() const override;
        virtual bool IsWaiting() const override;
        virtual void SetWaiting(bool waiting) override;
        virtual void CheckQuery(infra::BoundedString& hostname, DnsRecordPayload& payload, infra::ConstByteRange data) override;

        void Ask();

    private:
        MdnsClient& mdnsClient;
        services::DnsType dnsType;
        infra::BoundedString::WithStorage<253> dnsHostname;
        infra::Function<void(infra::ConstByteRange data)> queryHit;
        bool waiting = false;
    };

    class MdnsClient
        : private DatagramExchangeObserver
    {
    public:
        MdnsClient(DatagramFactory& datagramFactory, Multicast& multicast);
        ~MdnsClient();

        void RegisterQuery(MdnsQuery& query);
        void UnRegisterQuery(MdnsQuery& query);
        void ActiveQuerySingleShot(MdnsQuery& query);

    private:
        // Implementation of DatagramExchangeObserver
        virtual void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from) override;
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

        infra::detail::IntrusiveListIterator<MdnsQuery> FindNextWaitingQuery();
        void TrySendNextQuery();
        void SendQuery(MdnsQuery& query);
        void ActiveQueryDone();
        bool IsActivelyQuerying();
        void CancelActiveQueryIfEqual(MdnsQuery& query);
        
    private:
        class ActiveMdnsQuery
            : private DatagramExchangeObserver
        {
        public:
            ActiveMdnsQuery(MdnsClient& mdnsClient, DatagramFactory& datagramFactory, Multicast& multicast, MdnsQuery& query);
            ~ActiveMdnsQuery();

            bool IsCurrentQuery(MdnsQuery& query);

        private:
            // Implementation of DatagramExchangeObserver
            virtual void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader, UdpSocket from) override;
            virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

            void SendQuery();

        private:
            MdnsClient& mdnsClient;
            infra::SharedPtr<DatagramExchange> datagramExchange;
            Multicast& multicast;
            MdnsQuery& query;
        };

        class AnswerParser
        {
        public:
            AnswerParser(MdnsClient& client, infra::StreamReaderWithRewinding& reader);

        private:
            void Parse();
            bool IsValidAnswer() const;
            void ReadAnswer();
            void ReadHostname();

        private:
            MdnsClient& client;
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
        infra::SharedPtr<DatagramExchange> datagramExchange;
        infra::Optional<ActiveMdnsQuery> activeMdnsQuery;
        infra::IntrusiveList<MdnsQuery> queries;
        size_t lastWaitingQueryPosition = 0;
    };
}

#endif
