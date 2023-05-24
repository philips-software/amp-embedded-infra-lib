#include "infra/stream/InputStream.hpp"
#include "infra/util/Base64.hpp"
#include <cassert>

namespace infra
{
    InputStream::InputStream(StreamReader& reader, StreamErrorPolicy& errorPolicy)
        : reader(reader)
        , errorPolicy(errorPolicy)
    {}

    bool InputStream::Empty() const
    {
        return reader.Empty();
    }

    std::size_t InputStream::Available() const
    {
        return reader.Available();
    }

    ConstByteRange InputStream::ContiguousRange(std::size_t max) const
    {
        return reader.ExtractContiguousRange(max);
    }

    ConstByteRange InputStream::PeekContiguousRange(std::size_t start) const
    {
        return reader.PeekContiguousRange(start);
    }

    void infra::InputStream::Consume(std::size_t amount)
    {
        while (amount > 0)
        {
            auto reduced = reader.ExtractContiguousRange(amount).size();
            amount -= reduced;
            if (reduced == 0)
            {
                errorPolicy.ReportResult(false);
                return;
            }
        }

        errorPolicy.ReportResult(true);
    }

    bool InputStream::Failed() const
    {
        return errorPolicy.Failed();
    }

    StreamReader& InputStream::Reader() const
    {
        return reader;
    }

    StreamErrorPolicy& InputStream::ErrorPolicy() const
    {
        return errorPolicy;
    }

    TextInputStream DataInputStream::operator>>(Text)
    {
        return TextInputStream(Reader(), ErrorPolicy());
    }

    DataInputStream& DataInputStream::operator>>(ByteRange data)
    {
        Reader().Extract(data, ErrorPolicy());
        return *this;
    }

    DataInputStream TextInputStream::operator>>(Data)
    {
        return DataInputStream(Reader(), ErrorPolicy());
    }

    TextInputStream TextInputStream::operator>>(Hex)
    {
        TextInputStream res(*this);
        res.isDecimal = false;
        return res;
    }

    TextInputStream TextInputStream::operator>>(Width width)
    {
        TextInputStream res(*this);
        res.width = width.width;
        return res;
    }

    TextInputStream& TextInputStream::operator>>(MemoryRange<char> text)
    {
        Reader().Extract(ReinterpretCastByteRange(text), ErrorPolicy());
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(BoundedString& v)
    {
        *this >> MemoryRange<char>(v.begin(), v.end());
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(char& c)
    {
        Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(int8_t& v)
    {
        int32_t v32;
        Read(v32);
        v = static_cast<int8_t>(v32);
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(int16_t& v)
    {
        int32_t v32;
        Read(v32);
        v = static_cast<int16_t>(v32);
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(int32_t& v)
    {
        Read(v);
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(int64_t& v)
    {
        Read(v);
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(uint8_t& v)
    {
        uint32_t v32;
        Read(v32);
        v = static_cast<uint8_t>(v32);
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(uint16_t& v)
    {
        uint32_t v32;
        Read(v32);
        v = static_cast<uint16_t>(v32);
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(uint32_t& v)
    {
        Read(v);
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(uint64_t& v)
    {
        Read(v);
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(float& v)
    {
        int32_t integer = 0;
        operator>>(integer);
        uint32_t frac = 0;
        int32_t div = (integer < 0) ? -1 : 1;
        infra::StreamReader& reader = Reader();

        char c = static_cast<char>(reader.Peek(ErrorPolicy()));
        if (c == '.')
        {
            reader.Extract(infra::MakeByteRange(c), ErrorPolicy());
            c = static_cast<char>(reader.Peek(ErrorPolicy()));
            while (c >= '0' && c <= '9')
            {
                div *= 10;
                frac = frac * 10 + static_cast<uint8_t>(c - '0');
                reader.Extract(infra::MakeByteRange(c), ErrorPolicy());
                c = static_cast<char>(reader.Peek(ErrorPolicy()));
            }
        }

        v = static_cast<float>(frac);
        v /= div;
        v += integer;
        return *this;
    }

    TextInputStream& TextInputStream::operator>>(const char* literal)
    {
        while (*literal != '\0')
        {
            char c;
            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
            ErrorPolicy().ReportResult(c == *literal);
            ++literal;
        }

        return *this;
    }

    void TextInputStream::SkipSpaces()
    {
        char c = static_cast<char>(Reader().Peek(ErrorPolicy()));
        while (c == ' ')
        {
            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
            c = static_cast<char>(Reader().Peek(ErrorPolicy()));
        }
    }

    void TextInputStream::Read(int32_t& v)
    {
        if (isDecimal)
            ReadAsDecimal(v);
        else
            ReadAsHex(v);
    }

    void TextInputStream::Read(int64_t& v)
    {
        if (isDecimal)
            ReadAsDecimal(v);
        else
            ReadAsHex(v);
    }

    void TextInputStream::Read(uint32_t& v)
    {
        if (isDecimal)
            ReadAsDecimal(v);
        else
            ReadAsHex(v);
    }

    void TextInputStream::Read(uint64_t& v)
    {
        if (isDecimal)
            ReadAsDecimal(v);
        else
            ReadAsHex(v);
    }

    void TextInputStream::ReadAsDecimal(int32_t& v)
    {
        SkipSpaces();
        char c = Reader().Peek(ErrorPolicy());

        if (c == '-')
        {
            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
            v = -1;
        }
        else
            v = 1;

        uint32_t vp = 0;
        ReadAsDecimal(vp);
        v *= vp;
    }

    void TextInputStream::ReadAsDecimal(int64_t& v)
    {
        SkipSpaces();
        char c = Reader().Peek(ErrorPolicy());

        if (c == '-')
        {
            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
            v = -1;
        }
        else
            v = 1;

        uint64_t vp = 0;
        ReadAsDecimal(vp);
        v *= vp;
    }

    void TextInputStream::ReadAsDecimal(uint32_t& v)
    {
        SkipSpaces();

        v = 0;
        for (std::size_t i = 0; (i != width.ValueOr(std::numeric_limits<std::size_t>::max()) && !Reader().Empty()) || i == 0; ++i)
        {
            char c = static_cast<char>(Reader().Peek(ErrorPolicy()));

            if (c >= '0' && c <= '9')
                v = v * 10 + static_cast<uint32_t>(c - '0');
            else
            {
                ErrorPolicy().ReportResult(i > 0);
                break;
            }

            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
        }
    }

    void TextInputStream::ReadAsDecimal(uint64_t& v)
    {
        SkipSpaces();

        v = 0;
        for (std::size_t i = 0; (i != width.ValueOr(std::numeric_limits<std::size_t>::max()) && !Reader().Empty()) || i == 0; ++i)
        {
            char c = static_cast<char>(Reader().Peek(ErrorPolicy()));

            if (c >= '0' && c <= '9')
                v = v * 10 + static_cast<uint64_t>(c - '0');
            else
            {
                ErrorPolicy().ReportResult(i > 0);
                break;
            }

            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
        }
    }

    void TextInputStream::ReadAsHex(int32_t& v)
    {
        SkipSpaces();
        char c = static_cast<char>(Reader().Peek(ErrorPolicy()));
        if (c == '-')
        {
            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
            v = -1;
        }
        else
            v = 1;

        uint32_t vp = 0;
        ReadAsHex(vp);
        v *= vp;
    }

    void TextInputStream::ReadAsHex(int64_t& v)
    {
        SkipSpaces();
        char c = static_cast<char>(Reader().Peek(ErrorPolicy()));
        if (c == '-')
        {
            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
            v = -1;
        }
        else
            v = 1;

        uint64_t vp = 0;
        ReadAsHex(vp);
        v *= vp;
    }

    void TextInputStream::ReadAsHex(uint32_t& v)
    {
        SkipSpaces();

        v = 0;

        for (std::size_t i = 0; (i != width.ValueOr(std::numeric_limits<std::size_t>::max()) && !Reader().Empty()) || i == 0; ++i)
        {
            char c = static_cast<char>(Reader().Peek(ErrorPolicy()));

            if (c >= '0' && c <= '9')
                v = (v << 4) + static_cast<uint32_t>(c - '0');
            else if (c >= 'a' && c <= 'f')
                v = (v << 4) + static_cast<uint32_t>(c - 'a') + 10;
            else if (c >= 'A' && c <= 'F')
                v = (v << 4) + static_cast<uint32_t>(c - 'A') + 10;
            else
            {
                ErrorPolicy().ReportResult(i > 0);
                break;
            }

            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
        }
    }

    void TextInputStream::ReadAsHex(uint64_t& v)
    {
        SkipSpaces();

        v = 0;

        for (std::size_t i = 0; (i != width.ValueOr(std::numeric_limits<std::size_t>::max()) && !Reader().Empty()) || i == 0; ++i)
        {
            char c = static_cast<char>(Reader().Peek(ErrorPolicy()));

            if (c >= '0' && c <= '9')
                v = (v << 4) + static_cast<uint64_t>(c - '0');
            else if (c >= 'a' && c <= 'f')
                v = (v << 4) + static_cast<uint64_t>(c - 'a') + 10;
            else if (c >= 'A' && c <= 'F')
                v = (v << 4) + static_cast<uint64_t>(c - 'A') + 10;
            else
            {
                ErrorPolicy().ReportResult(i > 0);
                break;
            }

            Reader().Extract(infra::MakeByteRange(c), ErrorPolicy());
        }
    }

    namespace
    {
        uint8_t DecodeHexByte(char hex)
        {
            if (hex >= '0' && hex <= '9')
                return hex - '0';
            if (hex >= 'a' && hex <= 'f')
                return hex - 'a' + 10;
            if (hex >= 'A' && hex <= 'F')
                return hex - 'A' + 10;
            return 16;
        }
    }

    FromHexHelper::FromHexHelper(infra::ByteRange data)
        : data(data)
    {}

    infra::TextInputStream& operator>>(infra::TextInputStream& stream, FromHexHelper helper)
    {
        uint8_t bitIndex = 0;
        uint8_t decodedByte = 0;

        while (!helper.data.empty() && !stream.Empty())
        {
            char currentChar;
            stream >> currentChar;

            uint8_t decodedValue = DecodeHexByte(currentChar);
            ++currentChar;

            if (decodedValue >= 16)
            {
                stream.ErrorPolicy().ReportResult(false);
                return stream;
            }

            decodedByte = (decodedByte << 4) | decodedValue;
            bitIndex += 4;

            if (bitIndex == 8)
            {
                helper.data.front() = decodedByte;
                helper.data.pop_front();

                bitIndex = 0;
            }
        }

        stream.ErrorPolicy().ReportResult(helper.data.empty());
        return stream;
    }

    infra::TextInputStream& operator>>(infra::TextInputStream&& stream, FromHexHelper helper)
    {
        return stream >> helper;
    }

    FromBase64Helper::FromBase64Helper(infra::ByteRange data)
        : data(data)
    {}

    infra::TextInputStream& operator>>(infra::TextInputStream& stream, FromBase64Helper helper)
    {
        uint8_t bitIndex = 0;
        uint16_t decodedWord = 0;

        while (!helper.data.empty() && !stream.Empty())
        {
            char currentChar;
            stream >> currentChar;

            uint8_t decodedValue = DecodeBase64Byte(currentChar);
            ++currentChar;

            if (decodedValue >= 64)
            {
                stream.ErrorPolicy().ReportResult(false);
                return stream;
            }

            decodedWord = (decodedWord << 6) | decodedValue;
            bitIndex += 6;

            if (bitIndex >= 8)
            {
                helper.data.front() = static_cast<uint8_t>(decodedWord >> static_cast<uint8_t>(bitIndex - 8));
                helper.data.pop_front();

                bitIndex -= 8;
            }
        }

        while (bitIndex != 0 && !stream.Empty())
        {
            char currentChar;
            stream >> currentChar;
            stream.ErrorPolicy().ReportResult(currentChar == '=');
            bitIndex -= 2;
        }

        stream.ErrorPolicy().ReportResult(helper.data.empty());
        return stream;
    }

    infra::TextInputStream& operator>>(infra::TextInputStream&& stream, FromBase64Helper helper)
    {
        return stream >> helper;
    }

    FromHexHelper FromHex(infra::ByteRange data)
    {
        return FromHexHelper(data);
    }

    FromBase64Helper FromBase64(infra::ByteRange data)
    {
        return FromBase64Helper(data);
    }

    std::size_t Base64DecodedSize(infra::BoundedConstString encodedContents)
    {
        if (encodedContents.size() < 4)
            return 0;

        auto size = encodedContents.size() / 4 * 3;
        for (auto it = encodedContents.crbegin(); *it == '=' && it != encodedContents.crend(); ++it)
            --size;

        return size;
    }
}
