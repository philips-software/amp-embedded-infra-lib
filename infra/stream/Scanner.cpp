#include "infra/stream/Scanner.hpp"

namespace infra
{
    ScanSpec::ScanSpec(const char*& format)
        : format(format)
    {
        if (*format != ':')
            return;

        ++format;
        ParseWidth();
        ParseType();
    }

    void ScanSpec::ParseWidth()
    {
        if (!isdigit(*format))
            return;
        width = 0;
        while (isdigit(*format))
            width = 10 * width + *format++ - '0';
    }

    void ScanSpec::ParseType()
    {
        if (*format != '}')
            type = *format++;
    }

    void ScannerBase::SkipWhiteSpace(TextInputStream& stream)
    {
        const auto peek = stream.Reader().PeekContiguousRange(0);
        size_t skipped = 0;
        while (skipped < peek.size() && isspace(peek[skipped]))
            skipped++;
        stream.Consume(skipped);
    }

    bool ScannerBase::IsNegative(TextInputStream& stream)
    {
        const auto peek = stream.Reader().PeekContiguousRange(0);
        if (peek.empty())
            return false;

        const auto c = peek[0];
        if (c != '-' && c != '+')
            return false;

        stream.Consume(1);
        return c == '-';
    }

    uint64_t ScannerBase::RawInteger(TextInputStream& stream, ScanSpec& spec)
    {
        const auto peek = stream.Reader().PeekContiguousRange(0);
        const auto maxIndex = std::min(peek.size(), spec.width);
        size_t index{};

        uint64_t value{};

        uint8_t base;
        switch (spec.type)
        {
        case 'x':
            base = 16;
            break;
        case 'o':
            base = 8;
            break;
        case 'b':
            base = 2;
            break;
        case 'd':
        case '\0':
            base = 10;
            break;
        default:
            stream.ErrorPolicy().ReportResult(false);
            return 0;
        }

        const auto handleChar = [&value, base](uint8_t ch)
        {
            const auto digits = "0123456789abcdef";
            ch = tolower(ch);
            for (decltype(base) i = 0u; i<base; ++i)
                if (digits[i] == ch)
                {
                    value = value * base + i;
                    return true;
                }
            return false;
        };

        while (index < maxIndex && handleChar(peek[index]))
            index++;

        stream.Consume(index);
        return value;
    }

    int64_t ScannerBase::SignedInteger(TextInputStream& stream, ScanSpec& spec)
    {
        SkipWhiteSpace(stream);
        const auto negative = IsNegative(stream);
        const  auto val = RawInteger(stream, spec);
        return negative ? -static_cast<int64_t>(val) : val;
    }

    uint64_t ScannerBase::UnsignedInteger(TextInputStream& stream, ScanSpec& spec)
    {
        SkipWhiteSpace(stream);
        return RawInteger(stream, spec);
    }

    template<>
    void Scanner<bool>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        const auto handle = [&stream, this](const char* compare, const bool val)
        {
            auto peek = stream.Reader().PeekContiguousRange(0);
            const auto size = strlen(compare);
            peek.shrink_from_back_to(size);

            if (peek.size() < size)
                return false;
            for (auto c: peek)
                if (c != *compare++)
                    return false;

            stream.Consume(size);
            value = val;
            return true;
        };

        SkipWhiteSpace(stream);
        stream.ErrorPolicy().ReportResult(handle("true", true) || handle("false", false));
    }

    template<>
    void Scanner<uint8_t>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        value = static_cast<uint8_t>(UnsignedInteger(stream, spec));
    }

    template<>
    void Scanner<uint16_t>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        value = static_cast<uint16_t>(UnsignedInteger(stream, spec));
    }

    template<>
    void Scanner<uint32_t>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        value = static_cast<uint32_t>(UnsignedInteger(stream, spec));
    }

    template<>
    void Scanner<uint64_t>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        value = UnsignedInteger(stream, spec);
    }

    template<>
    void Scanner<int8_t>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        value = static_cast<int8_t>(SignedInteger(stream, spec));;
    }

    template<>
    void Scanner<int16_t>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        value = static_cast<int16_t>(SignedInteger(stream, spec));;
    }

    template<>
    void Scanner<int32_t>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        value = static_cast<int32_t>(SignedInteger(stream, spec));;
    }

    template<>
    void Scanner<int64_t>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        value = SignedInteger(stream, spec);
    }

    template<>
    void Scanner<BoundedString>::Scan(TextInputStream& stream, ScanSpec& spec)
    {
        const auto peek = stream.Reader().PeekContiguousRange(0);
        const auto size = std::min(std::min(spec.width, value.max_size()), peek.size());

        value.append(reinterpret_cast<const char*>(peek.begin()), size);
        stream.Consume(size);
    }

    ScanWorker::ScanWorker(TextInputStream& stream, const char* formatStr, std::vector<ScannerBase*>& scanners)
        : format(formatStr)
    {
        auto ok = true;
        size_t index = 0;
        do
        {
            if (IsEndFormat())
                break;

            const auto ch = *format++;
            if (ch != '{')
            {
                char c;
                stream >> c;
                if (ch != c)
                    ok = false;
                continue;
            }
            auto spec = ScanSpec(format);

            if (*format++ == '}' && index < scanners.size())
                scanners[index++]->Scan(stream, spec);
            else
                ok = false;
        } while (ok);

        if (index != scanners.size())
            ok = false;

        stream.ErrorPolicy().ReportResult(ok);
    }

    bool ScanWorker::IsEndFormat() const
    {
        return *format == '\0';
    }
}
