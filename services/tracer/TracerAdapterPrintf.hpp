#ifndef SERVICES_TRACER_ADAPTER_PRINTF_HPP
#define SERVICES_TRACER_ADAPTER_PRINTF_HPP

#include "services/tracer/Tracer.hpp"
#include <cstdarg>

namespace services
{
    class TracerAdapterPrintf
    {
    public:
        explicit TracerAdapterPrintf(services::Tracer& tracer);

        void Print(const char* format, va_list* args);

    private:
        void HandleFormat(const char*& format, va_list* args);
        int ReadLength(const char*& format) const;
        infra::Width ReadSize(const char*& format) const;
        void ParseFormat(char format, int lengthSpecifier, const infra::Width& width, va_list* args);

    private:
        services::Tracer& tracer;
    };
}

#endif
