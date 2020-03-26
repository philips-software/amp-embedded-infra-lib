#include "services/network/Dns.hpp"
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

    DnsHostnamePartsStream::DnsHostnamePartsStream(infra::StreamReaderWithRewinding& reader, uint32_t streamPosition)
        : reader(reader)
        , streamPosition(streamPosition)
        , stream(reader, infra::noFail)
    {
        ReadNext();
    }

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
}

namespace infra
{
    DataOutputStream& operator<<(DataOutputStream& stream, services::DnsHostnamePartsString& dnsPartsString)
    {
        while (!dnsPartsString.Empty())
        {
            stream << static_cast<uint8_t>(dnsPartsString.Current().size()) << infra::text << dnsPartsString.Current();
            dnsPartsString.ConsumeCurrent();
        }

        stream << static_cast<uint8_t>(0);

        return stream;
    }
}
