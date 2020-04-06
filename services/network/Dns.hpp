#ifndef SERVICES_DNS_HPP
#define SERVICES_DNS_HPP

#include "infra/stream/InputStream.hpp"
#include "infra/stream/OutputStream.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/Endian.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Tokenizer.hpp"

namespace services
{
    enum class DnsType : uint16_t
    {
        dnsTypeA = 1,
        dnsTypeNameServer = 2,
        dnsTypeCName = 5,
        dnsTypeAAAA = 26
    };

    enum class DnsClass : uint16_t
    {
        dnsClassIn = 1,
        dnsClassCh = 3
    };

    struct DnsRecordHeader
    {
        static const uint16_t flagsOpcodeMask = 0x7800;
        static const uint16_t flagsOpcodeQuery = 0;
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

    struct DnsQuestionFooter
    {
        DnsQuestionFooter() = default;
        DnsQuestionFooter(DnsType type, DnsClass class_)
            : type(infra::enum_cast(type))
            , class_(infra::enum_cast(class_))
        {}

        infra::BigEndian<uint16_t> type;
        infra::BigEndian<uint16_t> class_;
    };

    struct DnsRecordPayload
    {
        DnsRecordPayload() = default;
        DnsRecordPayload(DnsType type, DnsClass class_, infra::Duration ttl, uint16_t dataLength)
            : type(infra::enum_cast(type))
            , class_(infra::enum_cast(class_))
            , resourceDataLength(dataLength)
        {
            uint32_t countInSeconds = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(ttl).count());
            this->ttl[0] = static_cast<uint16_t>((countInSeconds >> 16) & 0xffff);
            this->ttl[1] = static_cast<uint16_t>(countInSeconds & 0xffff);
        }

        infra::BigEndian<uint16_t> type;
        infra::BigEndian<uint16_t> class_;
        std::array<infra::BigEndian<uint16_t>, 2> ttl;
        infra::BigEndian<uint16_t> resourceDataLength;

        bool IsCName() const;
        bool IsIPv4Answer() const;
        bool IsNameServer() const;

        infra::Duration Ttl() const;
    };

    class DnsHostnameParts
    {
    protected:
        DnsHostnameParts() = default;
        DnsHostnameParts(const DnsHostnameParts& other) = delete;
        DnsHostnameParts& operator=(const DnsHostnameParts& other) = delete;
        ~DnsHostnameParts() = default;

    public:
        virtual infra::BoundedConstString Current() const = 0;
        virtual void ConsumeCurrent() = 0;
        virtual bool Empty() const = 0;
    };

    class DnsHostnamePartsString
        : public DnsHostnameParts
    {
    public:
        DnsHostnamePartsString(infra::BoundedConstString hostname);

        virtual infra::BoundedConstString Current() const override;
        virtual void ConsumeCurrent() override;
        virtual bool Empty() const override;

    private:
        infra::Tokenizer hostnameTokens;
        std::size_t currentToken = 0;
    };

    class DnsHostnamePartsStream
        : public DnsHostnameParts
    {
    public:
        DnsHostnamePartsStream(infra::StreamReaderWithRewinding& reader, uint32_t streamPosition);

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
}

namespace infra
{
    DataOutputStream& operator<<(DataOutputStream& stream, services::DnsHostnamePartsString& dnsPartsString);
}

#endif
