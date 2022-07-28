#include "infra/stream/SavedMarkerStream.hpp"
#include "infra/syntax/ProtoFormatter.hpp"

namespace infra
{
    ProtoLengthDelimitedFormatter::ProtoLengthDelimitedFormatter(ProtoFormatter& formatter, uint32_t fieldNumber)
        : formatter(formatter)
    {
        formatter.PutVarInt((fieldNumber << 3) | 2);
        marker = formatter.output.SaveMarker();
    }

    ProtoLengthDelimitedFormatter::ProtoLengthDelimitedFormatter(ProtoLengthDelimitedFormatter&& other)
        : formatter(other.formatter)
        , marker(other.marker)
    {
        other.marker = std::numeric_limits<std::size_t>::max();
    }

    ProtoLengthDelimitedFormatter::~ProtoLengthDelimitedFormatter()
    {
        if (marker != std::numeric_limits<std::size_t>::max())
        {
            uint32_t size = formatter.output.ProcessedBytesSince(marker);
            infra::SavedMarkerDataStream savedStream(formatter.output, marker);
            ProtoFormatter savedFormatter(savedStream);
            savedFormatter.PutVarInt(size);
        }
    }

    ProtoFormatter::ProtoFormatter(infra::DataOutputStream& output)
        : output(output.Writer(), output.ErrorPolicy())
    {}

    void ProtoFormatter::PutVarInt(uint64_t value)
    {
        do
        {
            if (value > 127)
                output << static_cast<uint8_t>((value & 0x7f) | 0x80);
            else
                output << static_cast<uint8_t>(value);

            value >>= 7;
        } while (value != 0);
    }

    void ProtoFormatter::PutSignedVarInt(uint64_t value)
    {
        static const bool isArithmeticRightShift = (static_cast<signed int>(-1) >> 1) == static_cast<signed int>(-1);
        static_assert(isArithmeticRightShift, "");
        PutVarInt((value << 1) ^ (static_cast<int64_t>(value) >> 63));
    }

    void ProtoFormatter::PutFixed32(uint32_t value)
    {
        output << value;
    }

    void ProtoFormatter::PutFixed64(uint64_t value)
    {
        output << value;
    }

    void ProtoFormatter::PutString(infra::BoundedConstString string)
    {
        PutVarInt(string.size());
        output << infra::StringAsByteRange(string);
    }

    void ProtoFormatter::PutBytes(infra::ConstByteRange bytes)
    {
        PutVarInt(bytes.size());
        output << bytes;
    }

    void ProtoFormatter::PutVarIntField(uint64_t value, uint32_t fieldNumber)
    {
        PutVarInt((fieldNumber << 3) | 0);
        PutVarInt(value);
    }

    void ProtoFormatter::PutSignedVarIntField(uint64_t value, uint32_t fieldNumber)
    {
        PutVarInt((fieldNumber << 3) | 0);
        PutSignedVarInt(value);
    }

    void ProtoFormatter::PutFixed32Field(uint32_t value, uint32_t fieldNumber)
    {
        PutVarInt((fieldNumber << 3) | 5);
        PutFixed32(value);
    }

    void ProtoFormatter::PutFixed64Field(uint64_t value, uint32_t fieldNumber)
    {
        PutVarInt((fieldNumber << 3) | 1);
        PutFixed64(value);
    }

    void ProtoFormatter::PutLengthDelimitedField(infra::ConstByteRange range, uint32_t fieldNumber)
    {
        PutVarInt((fieldNumber << 3) | 2);
        PutVarInt(range.size());
        output << range;
    }

    void ProtoFormatter::PutStringField(infra::BoundedConstString string, uint32_t fieldNumber)
    {
        PutVarInt((fieldNumber << 3) | 2);
        PutString(string);
    }

    void ProtoFormatter::PutBytesField(infra::ConstByteRange bytes, uint32_t fieldNumber)
    {
        PutVarInt((fieldNumber << 3) | 2);
        PutBytes(bytes);
    }

    ProtoLengthDelimitedFormatter ProtoFormatter::LengthDelimitedFormatter(uint32_t fieldNumber)
    {
        return ProtoLengthDelimitedFormatter(*this, fieldNumber);
    }
}
