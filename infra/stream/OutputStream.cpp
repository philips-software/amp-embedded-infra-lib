#include "infra/stream/OutputStream.hpp"
#include <cmath>
#include <limits>

namespace infra
{
    std::size_t StreamWriter::ConstructSaveMarker() const
    {
        std::abort();
    }

    std::size_t StreamWriter::GetProcessedBytesSince(std::size_t marker) const
    {
        std::abort();
    }

    ByteRange StreamWriter::SaveState(std::size_t marker)
    {
        std::abort();
    }

    void StreamWriter::RestoreState(ByteRange range)
    {
        std::abort();
    }

    ByteRange StreamWriter::Overwrite(std::size_t marker)
    {
        std::abort();
    }

    StreamWriterDummy::StreamWriterDummy()
    {}

    void StreamWriterDummy::Insert(ConstByteRange range, StreamErrorPolicy& errorPolicy)
    {}

    std::size_t StreamWriterDummy::Available() const
    {
        return std::numeric_limits<std::size_t>::max();
    }

    OutputStream::OutputStream(StreamWriter& writer, StreamErrorPolicy& errorPolicy)
        : writer(writer)
        , errorPolicy(errorPolicy)
    {}

    bool OutputStream::Failed() const
    {
        return errorPolicy.Failed();
    }

    std::size_t OutputStream::SaveMarker() const
    {
        return writer.ConstructSaveMarker();
    }

    std::size_t OutputStream::ProcessedBytesSince(std::size_t marker) const
    {
        return writer.GetProcessedBytesSince(marker);
    }

    std::size_t OutputStream::Available() const
    {
        return writer.Available();
    }
    
    StreamWriter& OutputStream::Writer() const
    {
        return writer;
    }
    
    StreamErrorPolicy& OutputStream::ErrorPolicy() const
    {
        return errorPolicy;
    }

    TextOutputStream DataOutputStream::operator<<(Text)
    {
        return TextOutputStream(Writer(), ErrorPolicy());
    }

    TextOutputStream& TextOutputStream::operator<<(const char* zeroTerminatedString)
    {
        const auto len = std::strlen(zeroTerminatedString);
        OutputOptionalPadding(len);
        Writer().Insert(ReinterpretCastByteRange(MakeRange(zeroTerminatedString, zeroTerminatedString + len)), ErrorPolicy());
        return *this;
    }

    TextOutputStream& TextOutputStream::operator<<(BoundedConstString string)
    {
        Writer().Insert(ReinterpretCastByteRange(MakeRange(string.begin(), string.end())), ErrorPolicy());

        return *this;
    }

    TextOutputStream& TextOutputStream::operator<<(const std::string& string)
    {
        Writer().Insert(ReinterpretCastByteRange(MakeRange(string.data(), string.data() + string.size())), ErrorPolicy());

        return *this;
    }

    DataOutputStream TextOutputStream::operator<<(Data)
    {
        return DataOutputStream(Writer(), ErrorPolicy());
    }

    TextOutputStream::TextOutputStream(StreamWriter& writer, StreamErrorPolicy& errorPolicy)
        : OutputStream(writer, errorPolicy)
    {}

    TextOutputStream TextOutputStream::operator<<(Hex)
    {
        TextOutputStream result(*this);
        result.radix = Radix::hex;
        return result;
    }

    TextOutputStream TextOutputStream::operator<<(Bin)
    {
        TextOutputStream result(*this);
        result.radix = Radix::bin;
        return result;
    }

    TextOutputStream TextOutputStream::operator<<(Width width)
    {
        TextOutputStream result(*this);
        result.width = width;
        return result;
    }

    TextOutputStream& TextOutputStream::operator<<(Endl)
    {
        *this << "\r\n";
        return *this;
    }

    TextOutputStream& TextOutputStream::operator<<(char c)
    {
        Writer().Insert(MakeByteRange(c), ErrorPolicy());
        return *this;
    }

    TextOutputStream& TextOutputStream::operator<<(uint8_t v)
    {
        return *this << static_cast<uint64_t>(v);
    }

    TextOutputStream& TextOutputStream::operator<<(int32_t v)
    {
        return *this << static_cast<int64_t>(v);
    }

    TextOutputStream& TextOutputStream::operator<<(uint32_t v)
    {
        return *this << static_cast<uint64_t>(v);
    }

    TextOutputStream& TextOutputStream::operator<<(int64_t v)
    {
        const auto negative = v < 0;
        if (negative)
            v = -v;
        switch (radix)
        {
            case Radix::dec:
                OutputAsDecimal(v, negative);
                break;
            case Radix::bin:
                OutputAsBinary(v, negative);
                break;
            case Radix::hex:
                OutputAsHexadecimal(v, negative);
                break;
            default:
                std::abort();
        }

        return *this;
    }

    TextOutputStream& TextOutputStream::operator<<(uint64_t v)
    {
        switch (radix)
            {
            case Radix::dec:
                OutputAsDecimal(v, false);
                break;
            case Radix::bin:
                OutputAsBinary(v, false);
                break;
            case Radix::hex:
                OutputAsHexadecimal(v, false);
                break;
            default:
                std::abort();
        }

        return *this;
    }

#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ <= 9 && __GNUC_PATCHLEVEL__ < 4
    TextOutputStream& TextOutputStream::operator<<(int v)
    {
        return *this << static_cast<int64_t>(v);
    }

    TextOutputStream& TextOutputStream::operator<<(unsigned int v)
    {
        return *this << static_cast<uint64_t>(v);
    }
#endif

    TextOutputStream& TextOutputStream::operator<<(float v)
    {
        if (v < 0)
            *this << "-";

        v = std::abs(v);

        *this << static_cast<uint32_t>(v);
        v -= static_cast<uint32_t>(v);
        *this << ".";
        *this << Width(3,'0') << static_cast<uint32_t>(v * 1000);
        return *this;
    }

    void TextOutputStream::OutputAsDecimal(uint64_t v, bool negative)
    {
        size_t nofDigits = negative ? 2 : 1;
        uint64_t mask = 1;

        while (v / mask >= 10)
        {
            mask *= 10;
            ++nofDigits;
        }
        OutputOptionalPadding(nofDigits);
        if (negative)
            Writer().Insert(MakeByteRange('-'), ErrorPolicy());

        while (mask != 0)
        {
            Writer().Insert(MakeByteRange(static_cast<char>(((v / mask) % 10) + '0')), ErrorPolicy());
            mask /= 10;
        }
    }

    void TextOutputStream::OutputAsBinary(uint64_t v, bool negative)
    {
        size_t nofDigits = negative ? 2 : 1;
        uint64_t mask = 1;

        while (v / mask >= 2)
        {
            mask *= 2;
            ++nofDigits;
        }
        OutputOptionalPadding(nofDigits);
        if (negative)
            Writer().Insert(MakeByteRange('-'), ErrorPolicy());

        while (mask != 0)
        {
            Writer().Insert(MakeByteRange(static_cast<char>('0' + ((v / mask) & 1))), ErrorPolicy());
            mask /= 2;
        }
    }

    void TextOutputStream::OutputAsHexadecimal(uint64_t v, bool negative)
    {
        static const char hexChars[] = "0123456789abcdef";

        size_t nofDigits = negative ? 2 : 1;
        uint64_t mask = 1;

        while (v / mask >= 16)
        {
            mask *= 16;
            ++nofDigits;
        }
        OutputOptionalPadding(nofDigits);
        if (negative)
            Writer().Insert(MakeByteRange('-'), ErrorPolicy());

        while (mask != 0)
        {
            Writer().Insert(MakeByteRange(hexChars[(v / mask) % 16]), ErrorPolicy());
            mask /= 16;
        }
    }

    void TextOutputStream::FormatArgs(const char* format, MemoryRange<FormatterBase*> formatters)
    {
        while (*format != '\0')
        {
            const char* start = format;
            while (*format != '\0' && *format != '%')
                ++format;

            Writer().Insert(ReinterpretCastByteRange(MakeRange(start, format)), ErrorPolicy());

            if (*format == '%')
            {
                if (format[1] >= '1' && format[1] <= '9')
                {
                    if (format[2] == '%')
                    {
                        uint8_t index = format[1] - '1';
                        if (index < formatters.size())
                            formatters[index]->Stream(*this);

                        ++format;
                    }

                    ++format;
                }

                ++format;
            }
        }
    }

    void TextOutputStream::OutputOptionalPadding(size_t size)
    {
        for (auto i = size; i < width.width; ++i)
            Writer().Insert(MakeByteRange(width.padding), ErrorPolicy());
    }

    DataOutputStream::WithErrorPolicy::WithErrorPolicy(StreamWriter& writer)
        : DataOutputStream(writer, errorPolicy)
    {}

    DataOutputStream::WithErrorPolicy::WithErrorPolicy(StreamWriter& writer, SoftFail)
        : DataOutputStream(writer, errorPolicy)
        , errorPolicy(softFail)
    {}

    DataOutputStream::WithErrorPolicy::WithErrorPolicy(StreamWriter& writer, NoFail)
        : DataOutputStream(writer, errorPolicy)
        , errorPolicy(softFail)
    {}

    DataOutputStream::WithErrorPolicy::WithErrorPolicy(const WithErrorPolicy& other)
        : DataOutputStream(other.Writer(), errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    TextOutputStream::WithErrorPolicy::WithErrorPolicy(StreamWriter& writer)
        : TextOutputStream(writer, errorPolicy)
    {}

    TextOutputStream::WithErrorPolicy::WithErrorPolicy(StreamWriter& writer, SoftFail)
        : TextOutputStream(writer, errorPolicy)
        , errorPolicy(softFail)
    {}

    TextOutputStream::WithErrorPolicy::WithErrorPolicy(StreamWriter& writer, NoFail)
        : TextOutputStream(writer, errorPolicy)
        , errorPolicy(noFail)
    {}

    TextOutputStream::WithErrorPolicy::WithErrorPolicy(const WithErrorPolicy& other)
        : TextOutputStream(other.Writer(), errorPolicy)
        , errorPolicy(other.ErrorPolicy())
    {}

    AsAsciiHelper::AsAsciiHelper(ConstByteRange data)
        : data(data)
    {}

    TextOutputStream& operator<<(TextOutputStream& stream, const AsAsciiHelper& asAsciiHelper)
    {
        for (uint8_t byte : asAsciiHelper.data)
            if (byte < 32 || byte >= 128)
                stream << '.';
            else
                stream << static_cast<char>(byte);

        return stream;
    }

    AsHexHelper::AsHexHelper(ConstByteRange data)
        : data(data)
    {}

    TextOutputStream& operator<<(TextOutputStream& stream, const AsHexHelper& asHexHelper)
    {
        TextOutputStream hexStream = stream << hex << Width(2, '0');

        for (uint8_t byte : asHexHelper.data)
            hexStream << byte;

        return stream;
    }

    AsBase64Helper::AsBase64Helper(ConstByteRange data)
        : data(data)
    {}

    TextOutputStream& operator<<(TextOutputStream& stream, const AsBase64Helper& asHexHelper)
    {
        static const char* encodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        uint8_t bitIndex = 2;                                                                                           //TICS !OLC#006
        uint8_t encodedByte = 0;
        uint32_t size = 0;

        for (uint8_t byte : asHexHelper.data)
        {
            encodedByte |= byte >> bitIndex;
            stream << encodeTable[encodedByte];
            ++size;

            encodedByte = static_cast<uint8_t>(byte << (8 - bitIndex)) >> 2;                                            //TICS !POR#006

            bitIndex += 2;

            if (bitIndex == 8)
            {
                stream << encodeTable[encodedByte];
                ++size;
                encodedByte = 0;
                bitIndex = 2;
            }
        }

        if ((size & 3) != 0)
        {
            stream << encodeTable[encodedByte];
            ++size;
        }
        if ((size & 3) != 0)
        {
            stream << '=';
            ++size;
        }
        if ((size & 3) != 0)
        {
            stream << '=';
            ++size;
        }

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream&& stream, const AsBase64Helper& asBase64Helper)
    {
        return stream << asBase64Helper;
    }

    AsAsciiHelper AsAscii(ConstByteRange data)
    {
        return AsAsciiHelper(data);
    }

    AsHexHelper AsHex(ConstByteRange data)
    {
        return AsHexHelper(data);
    }

    AsBase64Helper AsBase64(ConstByteRange data)
    {
        return AsBase64Helper(data);
    }
}
