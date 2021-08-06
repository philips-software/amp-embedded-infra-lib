#include "infra/syntax/ProtoParser.hpp"

namespace infra
{
    ProtoLengthDelimited::ProtoLengthDelimited(infra::DataInputStream inputStream, uint32_t length)
        : limitedReader(inputStream.Reader(), length)
        , input(limitedReader, inputStream.ErrorPolicy())
    {
        inputStream.ErrorPolicy().ReportResult(inputStream.Available() >= length);
    }

    ProtoLengthDelimited::ProtoLengthDelimited(const ProtoLengthDelimited& other)
        : limitedReader(other.limitedReader)
        , input(limitedReader, other.input.ErrorPolicy())
    {}

    void ProtoLengthDelimited::SkipEverything()
    {
        while (!input.Empty())
            input.ContiguousRange();
    }

    ProtoParser ProtoLengthDelimited::Parser()
    {
        return ProtoParser(input);
    }

    void ProtoLengthDelimited::GetString(infra::BoundedString& string)
    {
        string.resize(std::min(input.Available(), string.max_size()));
        input.ErrorPolicy().ReportResult(string.size() == input.Available());
        input >> infra::StringAsByteRange(string);
    }

    void ProtoLengthDelimited::GetStringReference(infra::BoundedConstString& string)
    {
        string = infra::ByteRangeAsString(input.ContiguousRange());
    }

    std::string ProtoLengthDelimited::GetStdString()
    {
        std::string result(input.Available(), ' ');
        input >> infra::ConstCastByteRange(infra::ConstByteRange(reinterpret_cast<const uint8_t*>(result.data()), reinterpret_cast<const uint8_t*>(result.data() + result.size())));
        return result;
    }

    void ProtoLengthDelimited::GetBytes(infra::BoundedVector<uint8_t>& bytes)
    {
        bytes.resize(std::min(input.Available(), bytes.max_size()));
        input.ErrorPolicy().ReportResult(bytes.size() == input.Available());
        input >> infra::MakeRange(bytes);
    }

    void ProtoLengthDelimited::GetBytesReference(infra::ConstByteRange& bytes)
    {
        bytes = input.ContiguousRange();
    }

    ProtoParser::ProtoParser(infra::DataInputStream inputStream)
        : limitedReader(inputStream.Reader(), inputStream.Available())
        , input(limitedReader, inputStream.ErrorPolicy())
    {}

    bool ProtoParser::Empty() const
    {
        return input.Empty();
    }

    uint64_t ProtoParser::GetVarInt()
    {
        uint64_t result = 0;
        uint8_t byte = 0;
        uint8_t shift = 0;

        do
        {
            input >> byte;

            result += static_cast<uint64_t>(byte & 0x7f) << shift;
            shift += 7;
        } while (!input.Failed() && (byte & 0x80) != 0);

        return result;
    }

    uint32_t ProtoParser::GetFixed32()
    {
        uint32_t result = 0;
        input >> result;
        return result;
    }

    uint64_t ProtoParser::GetFixed64()
    {
        uint64_t result = 0;
        input >> result;
        return result;
    }

    ProtoParser::Field ProtoParser::GetField()
    {
        uint32_t x = static_cast<uint32_t>(GetVarInt());
        uint8_t type = x & 7;
        uint32_t fieldNumber = x >> 3;

        switch (type)
        {
            case 0:
                return std::make_pair(GetVarInt(), fieldNumber);
            case 1:
                return std::make_pair(GetFixed64(), fieldNumber);
            case 2:
                return std::make_pair(ProtoLengthDelimited(input, static_cast<uint32_t>(GetVarInt())), fieldNumber);
            case 5:
                return std::make_pair(GetFixed32(), fieldNumber);
            default:
                input.ErrorPolicy().ReportResult(false);
                return std::make_pair(static_cast<uint32_t>(0), 0);
        }
    }

    void ProtoParser::ReportResult(bool ok)
    {
        input.ErrorPolicy().ReportResult(ok);
    }
}
