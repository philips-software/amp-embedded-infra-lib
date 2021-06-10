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
        dnsTypePtr = 12,
        dnsTypeTxt = 16,
        dnsTypeAAAA = 28,
        dnsTypeSrv = 33,
        dnsTypeNsec = 47
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
        static const uint16_t flagsNoError = 0;

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
        virtual std::size_t StreamedSize() const = 0;

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const DnsHostnameParts& dnsParts);
        friend infra::TextOutputStream& operator<<(infra::TextOutputStream&& stream, const DnsHostnameParts& dnsParts);

    protected:
        friend class DnsPartsWithoutTerminationHelper;
        void Stream(infra::TextOutputStream& stream) const;
        virtual void StreamWithoutTermination(infra::TextOutputStream& stream) const = 0;
    };

    class DnsHostnamePartsString
        : public DnsHostnameParts
    {
    public:
        DnsHostnamePartsString(infra::BoundedConstString hostname);

        virtual infra::BoundedConstString Current() const override;
        virtual void ConsumeCurrent() override;
        virtual bool Empty() const override;
        virtual std::size_t StreamedSize() const override;
        std::size_t NumberOfRemainingTokens() const;

    protected:
        virtual void StreamWithoutTermination(infra::TextOutputStream& stream) const override;

    private:
        infra::Tokenizer hostnameTokens;
        std::size_t currentToken = 0;
    };

    class DnsHostnamePartsStream
        : public DnsHostnameParts
    {
    public:
        DnsHostnamePartsStream(infra::StreamReaderWithRewinding& reader, uint32_t streamPosition);
        DnsHostnamePartsStream(const DnsHostnamePartsStream& other);
        ~DnsHostnamePartsStream() = default;

        virtual infra::BoundedConstString Current() const override;
        virtual void ConsumeCurrent() override;
        virtual bool Empty() const override;
        virtual std::size_t StreamedSize() const override;
        void ConsumeStream();

    protected:
        virtual void StreamWithoutTermination(infra::TextOutputStream& stream) const override;

    private:
        void DestructiveStream(infra::TextOutputStream& stream);
        void ReadNext();

    private:
        infra::StreamReaderWithRewinding& reader;
        uint32_t streamPosition;
        infra::DataInputStream::WithErrorPolicy stream;
        infra::BoundedString::WithStorage<63> label;
        infra::Optional<std::size_t> finalPosition;
    };

    template<std::size_t S>
    class DnsHostnameInPartsHelper
        : public DnsHostnameParts
    {
    public:
        DnsHostnameInPartsHelper(infra::BoundedConstString part);
        DnsHostnameInPartsHelper(const std::array<infra::BoundedConstString, S - 1>& parts, infra::BoundedConstString part);

        virtual infra::BoundedConstString Current() const override;
        virtual void ConsumeCurrent() override;
        virtual bool Empty() const override;
        virtual std::size_t StreamedSize() const override;

        DnsHostnameInPartsHelper<S + 1> operator()(infra::BoundedConstString nextPart) const;

    protected:
        virtual void StreamWithoutTermination(infra::TextOutputStream& stream) const override;

    private:
        std::array<infra::BoundedConstString, S> parts;
        typename std::array<infra::BoundedConstString, S>::const_iterator current{ parts.begin() };
    };

    DnsHostnameInPartsHelper<1> DnsHostnameInParts(infra::BoundedConstString part);

    class DnsPartsWithoutTerminationHelper
    {
    public:
        DnsPartsWithoutTerminationHelper(const DnsHostnameParts& parts);

        friend infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const DnsPartsWithoutTerminationHelper& dnsParts);
        friend infra::TextOutputStream& operator<<(infra::TextOutputStream&& stream, const DnsPartsWithoutTerminationHelper& dnsParts);

    private:
        void Stream(infra::TextOutputStream& stream) const;

    private:
        const DnsHostnameParts& parts;
    };

    DnsPartsWithoutTerminationHelper DnsPartsWithoutTermination(const DnsHostnameParts& parts);
}

////    Implementation    ////

namespace services
{
    template<std::size_t S>
    DnsHostnameInPartsHelper<S>::DnsHostnameInPartsHelper(infra::BoundedConstString part)
    {
        static_assert(S == 1, "Size must be 1 in this constructor");
        parts.front() = part;
    }

    template<std::size_t S>
    DnsHostnameInPartsHelper<S>::DnsHostnameInPartsHelper(const std::array<infra::BoundedConstString, S - 1>& parts, infra::BoundedConstString part)
    {
        for (std::size_t i = 0; i != parts.size(); ++i)
            this->parts[i] = parts[i];

        this->parts.back() = part;
    }

    template<std::size_t S>
    infra::BoundedConstString DnsHostnameInPartsHelper<S>::Current() const
    {
        return *current;
    }

    template<std::size_t S>
    void DnsHostnameInPartsHelper<S>::ConsumeCurrent()
    {
        ++current;
    }

    template<std::size_t S>
    bool DnsHostnameInPartsHelper<S>::Empty() const
    {
        return current == parts.end();
    }

    template<std::size_t S>
    std::size_t DnsHostnameInPartsHelper<S>::StreamedSize() const
    {
        std::size_t result = 1;

        for (auto part : parts)
            result += part.size() + 1;

        return result;
    }

    template<std::size_t S>
    DnsHostnameInPartsHelper<S + 1> DnsHostnameInPartsHelper<S>::operator()(infra::BoundedConstString nextPart) const
    {
        return DnsHostnameInPartsHelper<S + 1>(parts, nextPart);
    }

    template<std::size_t S>
    void DnsHostnameInPartsHelper<S>::StreamWithoutTermination(infra::TextOutputStream& stream) const
    {
        for (auto part : parts)
            stream << infra::data << static_cast<uint8_t>(part.size()) << infra::text << part;
    }
}

#endif
