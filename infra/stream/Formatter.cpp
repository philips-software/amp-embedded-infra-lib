#include "infra/stream/Formatter.hpp"

namespace infra
{
    FormatSpec::FormatSpec(const char*& format)
        : format(format)
    {
        if (*format != ':')
            return;

        ++format;
        ParseAlign();
        ParseZero();
        ParseWidth();
        ParseType();
    }

    void FormatSpec::ParseAlign()
    {
        auto i = (*format == '\0') ? 0 : 1;
        do
        {
            switch (format[i])
            {
            case '<':
                align = FormatAlign::left;
                break;
            case '^':
                align = FormatAlign::center;
                break;
            case '>':
                align = FormatAlign::right;
                break;
            default:
                break;
            }

            if (align != FormatAlign::nothing)
            {
                if (i == 1)
                    fill = *format++;
                ++format;
                return;
            }
        } while (i-- > 0);
    }

    void FormatSpec::ParseZero()
    {
        if (*format != '0')
            return;

        if (align == FormatAlign::nothing)
            fill = '0';

        format++;
    }

    void FormatSpec::ParseWidth()
    {
        while (isdigit(*format))
            width = 10 * width + *format++ - '0';
    }

    void FormatSpec::ParseType()
    {
        if (*format != '}')
            type = *format++;
    }

    void FormatterBase::RawFormat(TextOutputStream& stream, const BoundedConstString& text, const FormatSpec& spec) const
    {
        auto width = std::max(spec.width, text.size());

        std::size_t count = 0;
        switch (spec.align)
        {
        case FormatAlign::right:
            count = width - text.size();
            break;
        case FormatAlign::center:
            count = (width - text.size()) / 2;
            break;
        default:
            break;
        }
        width -= count;
        for (; count > 0; --count)
            stream << spec.fill;

        stream << text;

        width -= text.size();
        for (; width > 0; --width)
            stream << spec.fill;
    }

    void FormatterBase::RawInteger(TextOutputStream& stream, uint64_t value, bool negative, FormatSpec& spec) const
    {
        const std::size_t SignSize = 1;
        const auto size = std::numeric_limits<uint64_t>::digits + SignSize;
        BoundedString::WithStorage<size> buffer;
        auto index(size-1);

        decltype(value) divider{};
        switch (spec.type)
        {
        case 'x':
        case 'X':
            divider = 16;
            break;
        case 'o':
            divider = 8;
            break;
        case 'b':
            divider = 2;
            break;
        case 'd':
        case '\0':
            divider = 10;
            break;
        default:
            break;
        }
        const auto isValid = divider != 0;
        stream.ErrorPolicy().ReportResult(isValid);
        if (!isValid)
            return;

        const auto digits = isupper(spec.type) ? "0123456789ABCDEF" : "0123456789abcdef";

        for (; value >= divider; value /= divider)
            buffer[index--] = digits[value % divider];
        buffer[index] = digits[value];

        if (negative)
            buffer[--index] = '-';

        if (spec.align == FormatAlign::nothing)
            spec.align = FormatAlign::right;

        RawFormat(stream, BoundedConstString(&buffer[index], size-index), spec);
    }

    void FormatterBase::SignedInteger(TextOutputStream& stream, int64_t value, FormatSpec& spec) const
    {
        const auto negative = value < 0;
        RawInteger(stream, negative ? -value : value, negative, spec);
    }

    void FormatterBase::UnsignedInteger(TextOutputStream& stream, uint64_t value, FormatSpec& spec) const
    {
        RawInteger(stream, value, false, spec);
    }

    template<>
    void Formatter<int64_t const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        SignedInteger(stream, value, spec);
    }

    template<>
    void Formatter<int32_t const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        SignedInteger(stream, value, spec);
    }

    template<>
    void Formatter<int16_t const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        SignedInteger(stream, value, spec);
    }

    template<>
    void Formatter<int8_t const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        SignedInteger(stream, value, spec);
    }

    template<>
    void Formatter<uint64_t const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        UnsignedInteger(stream, value, spec);
    }

    template<>
    void Formatter<uint32_t const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        UnsignedInteger(stream, value, spec);
    }

    template<>
    void Formatter<uint16_t const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        UnsignedInteger(stream, value, spec);
    }

    template<>
    void Formatter<uint8_t const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        UnsignedInteger(stream, value, spec);
    }

    template<>
    void Formatter<char const*>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        RawFormat(stream, value, spec);
    }

    template<>
    void Formatter<bool const>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        const auto str = value ? "true" : "false";
        RawFormat(stream, str, spec);
    }

    FormatWorker::FormatWorker(TextOutputStream& stream, const char* formatStr, std::vector<FormatterBase*>& formatters)
        : format(formatStr)
    {
        do
        {
            if (IsEndFormat())
                return;

            const auto ch = *format++;
            if (ch != '{')
            {
                stream << ch;
                continue;
            }

            const auto index = ParseIndex();
            auto spec = FormatSpec(format);

            const auto isValid = *format++ == '}' && index < formatters.size();
            stream.ErrorPolicy().ReportResult(isValid);
            if (isValid)
                formatters[index]->Format(stream, spec);
        } while (!stream.Failed());
    }

    bool FormatWorker::IsEndFormat() const
    {
        return *format == '\0';
    }

    uint32_t FormatWorker::ParseIndex()
    {
        if (!isdigit(*format))
            return autoIndex++;

        uint32_t index = 0;
        do
        {
            index = 10 * index + *format++ - '0';
        } while (isdigit(*format));

        return index;
    }
}
