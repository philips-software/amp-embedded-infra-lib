#include "infra/syntax/Asn1Formatter.hpp"

namespace
{
    void AddLength(infra::DataOutputStream& stream, uint32_t length)
    {
        if (length > 0xFF)
            stream << static_cast<uint8_t>(0x82) << static_cast<uint8_t>((length & 0xFF00) >> 8) << static_cast<uint8_t>(length & 0xFF);
        else if (length > 0x7F)
            stream << static_cast<uint8_t>(0x81) << static_cast<uint8_t>(length & 0xFF);
        else
            stream << static_cast<uint8_t>(length & 0xFF);
    }
}

namespace infra
{
    Asn1Formatter::Asn1Formatter(infra::DataOutputStream& stream)
        : stream(stream)
    {}

    void Asn1Formatter::Add(uint8_t value)
    {
        AddTagLengthValue(Tag::Integer, sizeof(uint8_t), value);
    }

    void Asn1Formatter::Add(uint32_t value)
    {
        value = (value << 16) | (value >> 16);
        value = ((value & 0x00ff00ff) << 8) | ((value & 0xff00ff00) >> 8);

        AddTagLengthValue(Tag::Integer, sizeof(uint32_t), value);
    }

    void Asn1Formatter::Add(int32_t value)
    {
        Add(static_cast<uint32_t>(value));
    }

    void Asn1Formatter::AddSerial(infra::ConstByteRange serial)
    {
        AddTagLengthValue(Tag::Integer, serial.size(), serial);
    }

    void Asn1Formatter::AddBigNumber(infra::ConstByteRange number)
    {
        while (!number.empty() && number.back() == 0)
            number.pop_back();

        if (!number.empty() && (number.back() & 0x80) != 0)
        {
            AddTagLength(Tag::Integer, number.size() + 1);

            stream << static_cast<uint8_t>(0x00);
            for (int i = number.size() - 1; i >= 0; --i)
                stream << number[i];
        }
        else
        {
            AddTagLength(Tag::Integer, number.size());

            for (int i = number.size() - 1; i >= 0; --i)
                stream << number[i];
        }
    }

    void Asn1Formatter::AddContextSpecific(uint8_t context, infra::ConstByteRange data)
    {
        AddTagLengthValue(Tag::Constructed | Tag::ContextSpecific | context, data.size(), data);
    }

    void Asn1Formatter::AddObjectId(infra::ConstByteRange oid)
    {
        AddTagLengthValue(Tag::Oid, oid.size(), oid);
    }

    void Asn1Formatter::AddBitString(infra::ConstByteRange string)
    {
        AddTagLength(Tag::BitString, string.size() + 1);

        // Next byte indicates the bits of padding added to
        // the data when the length of the data in bits is
        // not a multiple of 8.
        stream << static_cast<uint8_t>(0x00);
        stream << string;
    }

    void Asn1Formatter::AddPrintableString(infra::ConstByteRange string)
    {
        AddTagLengthValue(Tag::PrintableString, string.size(), string);
    }

    void Asn1Formatter::AddUtcTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
    {
        const uint8_t utcTimeSize = 13; // "YYMMDDhhmmssZ"
        AddTagLength(Tag::UtcTime, utcTimeSize);

        year -= 1900;
        if (year >= 50)
            year -= 100;

        stream << infra::text << infra::Width(2, '0') << static_cast<uint8_t>(year);
        stream << infra::text << infra::Width(2, '0') << month;
        stream << infra::text << infra::Width(2, '0') << day;
        stream << infra::text << infra::Width(2, '0') << hour;
        stream << infra::text << infra::Width(2, '0') << min;
        stream << infra::text << infra::Width(2, '0') << sec;
        stream << infra::text << 'Z';
    }

    void Asn1Formatter::AddTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
    {
        if (year >= 1950 && year < 2050)
            AddUtcTime(year, month, day, hour, min, sec);
        else
            AddGeneralizedTime(year, month, day, hour, min, sec);
    }

    void Asn1Formatter::AddGeneralizedTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
    {
        const uint8_t generalizedTimeSize = 15; // "YYYYMMDDhhmmssZ"
        AddTagLength(Tag::GeneralizedTime, generalizedTimeSize);

        stream << infra::text << infra::Width(4, '0') << year;
        stream << infra::text << infra::Width(2, '0') << month;
        stream << infra::text << infra::Width(2, '0') << day;
        stream << infra::text << infra::Width(2, '0') << hour;
        stream << infra::text << infra::Width(2, '0') << min;
        stream << infra::text << infra::Width(2, '0') << sec;
        stream << infra::text << 'Z';
    }

    Asn1ContainerFormatter Asn1Formatter::StartSequence()
    {
        stream << static_cast<uint8_t>(Tag::Constructed | Tag::Sequence);

        return Asn1ContainerFormatter(stream, stream.SaveMarker());
    }

    Asn1ContainerFormatter Asn1Formatter::StartSet()
    {
        stream << static_cast<uint8_t>(Tag::Constructed | Tag::Set);

        return Asn1ContainerFormatter(stream, stream.SaveMarker());
    }

    Asn1ContainerFormatter Asn1Formatter::StartContextSpecific(uint8_t context)
    {
        stream << static_cast<uint8_t>(Tag::Constructed | Tag::ContextSpecific | context);

        return Asn1ContainerFormatter(stream, stream.SaveMarker());
    }

    Asn1ContainerFormatter Asn1Formatter::StartBitString()
    {
        stream << static_cast<uint8_t>(Tag::BitString);
        auto marker = stream.SaveMarker();

        // Next byte indicates the bits of padding added to
        // the data when the length of the data in bits is
        // not a multiple of 8.
        stream << static_cast<uint8_t>(0x00);

        return Asn1ContainerFormatter(stream, marker);
    }

    bool Asn1Formatter::Failed() const
    {
        return stream.Failed();
    }

    infra::DataOutputStream& Asn1Formatter::Stream()
    {
        return stream;
    }

    void Asn1Formatter::AddTagLength(uint8_t tag, uint32_t length)
    {
        stream << tag;
        AddLength(stream, length);
    }

    Asn1ContainerFormatter::Asn1ContainerFormatter(infra::DataOutputStream& stream, std::size_t sizeMarker)
        : Asn1Formatter(stream)
        , sizeMarker(sizeMarker)
    {}

    Asn1ContainerFormatter::Asn1ContainerFormatter(Asn1ContainerFormatter&& other)
        : Asn1Formatter(Stream())
        , sizeMarker(other.sizeMarker)
    {
        other.sizeMarker = std::numeric_limits<std::size_t>::max();
    }

    Asn1ContainerFormatter& Asn1ContainerFormatter::operator=(Asn1ContainerFormatter&& other)
    {
        sizeMarker = other.sizeMarker;
        other.sizeMarker = std::numeric_limits<std::size_t>::max();

        return *this;
    }

    Asn1ContainerFormatter::~Asn1ContainerFormatter()
    {
        if (sizeMarker != std::numeric_limits<std::size_t>::max())
        {
            infra::SavedMarkerDataStream sizeStream(Stream(), sizeMarker);
            AddLength(sizeStream, Stream().ProcessedBytesSince(sizeMarker));
        }
    }
}
