#include "services/tracer/TracerAdapterPrintf.hpp"
#include "infra/util/Compatibility.hpp"
#include "infra/util/ReallyAssert.hpp"

namespace services
{
    TracerAdapterPrintf::TracerAdapterPrintf(services::Tracer& tracer)
        : tracer(tracer)
    {}

    void TracerAdapterPrintf::Print(const char* format, va_list* args)
    {
        assert(format != nullptr);
        assert(args != nullptr);

        for (; *format != 0; ++format)
        {
            if (*format == '%')
                HandleFormat(format, args);
            else if (*format == '\n')
                tracer.Trace();
            else
                tracer.Continue() << *format;
        }
    }

    void TracerAdapterPrintf::HandleFormat(const char*& format, va_list* args)
    {
        ++format;

        const auto width = ReadSize(format);
        const auto precision = ReadPrecision(format, args);
        const auto lengthSpecifier = ReadLength(format);
        ParseFormat(*format, lengthSpecifier, width, precision, args);
    }

    int TracerAdapterPrintf::ReadLength(const char*& format) const
    {
        auto lengthSpecifier = 0;

        while (*format == 'l')
        {
            ++lengthSpecifier;
            ++format;
        }

        return lengthSpecifier;
    }

    int TracerAdapterPrintf::ReadPrecision(const char*& format, va_list* args) const
    {
        if (*format != '.')
            return -1;

        ++format;

        if (*format == '*')
        {
            ++format;
            return va_arg(*args, int);
        }

        int precision = 0;
        while (*format >= '0' && *format <= '9')
        {
            precision = precision * 10 + *format - '0';
            ++format;
        }

        return precision;
    }

    infra::Width TracerAdapterPrintf::ReadSize(const char*& format) const
    {
        infra::Width w(0);

        if (*format == '0')
        {
            w.padding = '0';
            format++;
        }

        while (*format >= '0' && *format <= '9')
        {
            w.width = w.width * 10 + *format - '0';
            ++format;
        }

        return w;
    }

    void TracerAdapterPrintf::ParseFormat(char format, int lengthSpecifier, const infra::Width& width, int precision, va_list* args)
    {
        switch (format)
        {
            case '\0':
                break;
            case '%':
                tracer.Continue() << format;
                break;
            case 'c':
                really_assert(precision < 0);
                tracer.Continue() << static_cast<const char>(va_arg(*args, int32_t));
                break;
            case 's':
            {
                const auto* s = va_arg(*args, char*);
                if (s == nullptr)
                    tracer.Continue() << "(null)";
                else if (precision >= 0)
                {
                    for (int i = 0; i < precision && s[i] != '\0'; ++i)
                        tracer.Continue() << s[i];
                }
                else
                    tracer.Continue() << s;
                break;
            }
            case 'd':
            case 'i':
                really_assert(precision < 0);
                if (lengthSpecifier >= 2)
                    tracer.Continue() << va_arg(*args, int64_t);
                else
                    tracer.Continue() << va_arg(*args, int32_t);
                break;
            case 'u':
                really_assert(precision < 0);
                if (lengthSpecifier >= 2)
                    tracer.Continue() << va_arg(*args, uint64_t);
                else
                    tracer.Continue() << va_arg(*args, uint32_t);
                break;
            case 'p':
                really_assert(precision < 0);
                tracer.Continue() << "0x";
                EMIL_FALLTHROUGH;
            case 'X':
            case 'x':
                really_assert(precision < 0);
                if (lengthSpecifier >= 2)
                    tracer.Continue() << infra::hex << width << va_arg(*args, uint64_t);
                else
                    tracer.Continue() << infra::hex << width << va_arg(*args, uint32_t);
                break;
            case 'f':
                really_assert(precision < 0);
                tracer.Continue() << static_cast<float>(va_arg(*args, double));
                break;
            default:
                while (lengthSpecifier-- > 0)
                    tracer.Continue() << 'l';
                tracer.Continue() << format;
                break;
        }
    }
}
