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

    void FormatterBase::RawFormat(TextOutputStream& stream, const char* text, std::size_t length, const FormatSpec& spec) const
    {
        auto width = std::max(spec.width, length);

        std::size_t count = 0;
        switch (spec.align)
        {
        case FormatAlign::right:
            count = width - length;
            break;
        case FormatAlign::center:
            count = (width - length) / 2;
            break;
        default:
            break;
        }
        width -= count;
        for (; count > 0; --count)
            stream << spec.fill;

        width -= length;
        for (; length > 0; --length)
            stream << *text++;

        for (; width > 0; --width)
            stream << spec.fill;
    }

    void FormatterBase::RawInteger(uint64_t value, bool negative, uint8_t dotPos, TextOutputStream& stream, FormatSpec& spec) const
    {
        const std::size_t dotAndSignSize = 2;
        const auto size = std::numeric_limits<uint64_t>::digits + dotAndSignSize;
        char buffer[size];
        auto ptr = &buffer[size - 1];

        decltype(value) divider = 10;
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
        default:
            break;
        }

        const auto digits = isupper(spec.type) ? "0123456789ABCDEF" : "0123456789abcdef";

        for (; value >= divider; value /= divider)
        {
            *ptr-- = digits[value % divider];
            if (--dotPos == 0)
                *ptr-- = '.';
        }
        *ptr = digits[value];

        if (negative)
            *--ptr = '-';

        if (spec.align == FormatAlign::nothing)
            spec.align = FormatAlign::right;

        RawFormat(stream, ptr, &buffer[size - 1] - ptr + 1, spec);
    }

    void FormatterBase::SignedInteger(int64_t value, TextOutputStream& stream, FormatSpec& spec) const
    {
        const auto negative = value < 0;
        RawInteger(negative ? -value : value, negative, 0, stream, spec);
    }

    void FormatterBase::UnsignedInteger(uint64_t value, TextOutputStream& stream, FormatSpec& spec) const
    {
        RawInteger(value, false, 0, stream, spec);
    }

    template<>
    void Formatter<int64_t>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        SignedInteger(value, stream, spec);
    }

    template<>
    void Formatter<int32_t>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        SignedInteger(value, stream, spec);
    }

    template<>
    void Formatter<int16_t>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        SignedInteger(value, stream, spec);
    }

    template<>
    void Formatter<int8_t>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        SignedInteger(value, stream, spec);
    }

    template<>
    void Formatter<uint64_t>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        UnsignedInteger(value, stream, spec);
    }

    template<>
    void Formatter<uint32_t>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        UnsignedInteger(value, stream, spec);
    }

    template<>
    void Formatter<uint16_t>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        UnsignedInteger(value, stream, spec);
    }

    template<>
    void Formatter<uint8_t>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        UnsignedInteger(value, stream, spec);
    }

    template<>
    void Formatter<char const*>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        RawFormat(stream, value, std::strlen(value), spec);
    }

    template<>
    void Formatter<bool>::Format(TextOutputStream& stream, FormatSpec& spec)
    {
        const auto str = value ? "true" : "false";
        RawFormat(stream, str, std::strlen(str), spec);
    }

    FormatWorker::FormatWorker(TextOutputStream& writer, const char* formatStr, std::vector<FormatterBase*>& formatters)
        : format(formatStr)
    {
        for (;;)
        {
            if (IsEndFormat())
                return;

            const auto ch = *format++;
            if (ch != '{')
            {
                writer << ch;
                continue;
            }

            const auto index = ParseIndex();
            auto spec = FormatSpec(format);

            if (*format == '}' && index < formatters.size())
            {
                formatters[index]->Format(writer, spec);
                ++format;
            }
        }
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
