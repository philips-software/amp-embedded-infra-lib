#include "services/network/Dns.hpp"
#include "infra/stream/CountingOutputStream.hpp"
#include "infra/stream/StringInputStream.hpp"
#include "services/network/Address.hpp"
#include <chrono>

namespace services
{
    bool DnsRecordPayload::IsCName() const
    {
        return class_ == infra::enum_cast(DnsClass::dnsClassIn) && type == infra::enum_cast(DnsType::dnsTypeCName);
    }

    bool DnsRecordPayload::IsIPv4Answer() const
    {
        return class_ == infra::enum_cast(DnsClass::dnsClassIn) && type == infra::enum_cast(DnsType::dnsTypeA) && resourceDataLength == static_cast<uint16_t>(sizeof(IPv4Address));
    }

    bool DnsRecordPayload::IsNameServer() const
    {
        return class_ == infra::enum_cast(DnsClass::dnsClassIn) && type == infra::enum_cast(DnsType::dnsTypeNameServer);
    }

    infra::Duration DnsRecordPayload::Ttl() const
    {
        return std::chrono::seconds((ttl[0] << 16) + ttl[1]);
    }

    void DnsHostnameParts::Stream(infra::TextOutputStream& stream) const
    {
        StreamWithoutTermination(stream);

        stream << infra::data << static_cast<uint8_t>(0);
    }

    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const DnsHostnameParts& dnsParts)
    {
        dnsParts.Stream(stream);
        return stream;
    }

    infra::TextOutputStream& operator<<(infra::TextOutputStream&& stream, const DnsHostnameParts& dnsParts)
    {
        return stream << dnsParts;
    }

    DnsHostnamePartsString::DnsHostnamePartsString(infra::BoundedConstString hostname)
        : hostnameTokens(hostname, '.')
    {}

    infra::BoundedConstString DnsHostnamePartsString::Current() const
    {
        return hostnameTokens.Token(currentToken);
    }

    void DnsHostnamePartsString::ConsumeCurrent()
    {
        ++currentToken;
    }

    bool DnsHostnamePartsString::Empty() const
    {
        return currentToken == hostnameTokens.Size();
    }

    std::size_t DnsHostnamePartsString::StreamedSize() const
    {
        if (hostnameTokens.Size() == 0)
            return 0;

        return hostnameTokens.TokenAndRest(0).size() + 1;
    }

    std::size_t DnsHostnamePartsString::NumberOfRemainingTokens() const
    {
        return hostnameTokens.Size() - currentToken;
    }

    void DnsHostnamePartsString::StreamWithoutTermination(infra::TextOutputStream& stream) const
    {
        for (std::size_t i = 0; i != hostnameTokens.Size(); ++i)
            stream << infra::data << static_cast<uint8_t>(hostnameTokens.Token(i).size()) << infra::text << hostnameTokens.Token(i);
    }

    DnsHostnamePartsStream::DnsHostnamePartsStream(infra::StreamReaderWithRewinding& reader, uint32_t streamPosition)
        : reader(reader)
        , streamPosition(streamPosition)
        , stream(reader, infra::noFail)
    {
        ReadNext();
    }

    DnsHostnamePartsStream::DnsHostnamePartsStream(const DnsHostnamePartsStream& other)
        : reader(other.reader)
        , streamPosition(other.streamPosition)
        , stream(reader, infra::noFail)
        , label(other.label)
        , finalPosition(other.finalPosition)
    {}

    infra::BoundedConstString DnsHostnamePartsStream::Current() const
    {
        return label;
    }

    void DnsHostnamePartsStream::ConsumeCurrent()
    {
        ReadNext();
    }

    bool DnsHostnamePartsStream::Empty() const
    {
        return label.empty();
    }

    std::size_t DnsHostnamePartsStream::StreamedSize() const
    {
        infra::TextOutputStream::WithWriter<infra::CountingStreamWriter> stream;
        stream << *this;
        return stream.Writer().Processed();
    }

    void DnsHostnamePartsStream::ConsumeStream()
    {
        reader.Rewind(finalPosition.ValueOr(this->streamPosition));
    }

    void DnsHostnamePartsStream::StreamWithoutTermination(infra::TextOutputStream& stream) const
    {
        auto copy(*this);
        copy.DestructiveStream(stream);
    }

    void DnsHostnamePartsStream::DestructiveStream(infra::TextOutputStream& stream)
    {
        while (!Empty())
        {
            stream << infra::data << static_cast<uint8_t>(Current().size()) << infra::text << Current();
            ConsumeCurrent();
        }
    }

    void DnsHostnamePartsStream::ReadNext()
    {
        label.clear();

        auto save = reader.ConstructSaveMarker();
        reader.Rewind(streamPosition);

        if (!stream.Empty())
        {
            auto size = stream.Extract<uint8_t>();

            while ((size & 0xc0) == 0xc0)
            {
                uint8_t offsetHigh = size & 0x3f;
                auto offsetLow = stream.Extract<uint8_t>();
                uint16_t offset = offsetLow + (offsetHigh << 8);

                if (!finalPosition)
                    finalPosition = reader.ConstructSaveMarker();

                size = 0;

                auto currentPosition = reader.ConstructSaveMarker();
                if (currentPosition <= offset)
                    break;

                reader.Rewind(offset);

                size = stream.Extract<uint8_t>();
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

    DnsHostnameInPartsHelper<1> DnsHostnameInParts(infra::BoundedConstString part)
    {
        return DnsHostnameInPartsHelper<1>(part);
    }

    DnsPartsWithoutTerminationHelper::DnsPartsWithoutTerminationHelper(const DnsHostnameParts& parts)
        : parts(parts)
    {}

    void DnsPartsWithoutTerminationHelper::Stream(infra::TextOutputStream& stream) const
    {
        parts.StreamWithoutTermination(stream);
    }

    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const DnsPartsWithoutTerminationHelper& dnsParts)
    {
        dnsParts.Stream(stream);
        return stream;
    }

    infra::TextOutputStream& operator<<(infra::TextOutputStream&& stream, const DnsPartsWithoutTerminationHelper& dnsParts)
    {
        return stream << dnsParts;
    }

    DnsPartsWithoutTerminationHelper DnsPartsWithoutTermination(const DnsHostnameParts& parts)
    {
        return DnsPartsWithoutTerminationHelper(parts);
    }
}
