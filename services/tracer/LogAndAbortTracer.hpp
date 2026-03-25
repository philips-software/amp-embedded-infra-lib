#ifndef SERVICES_UTIL_LOGANDABORTTRACER_HPP
#define SERVICES_UTIL_LOGANDABORTTRACER_HPP

#include "infra/util/Function.hpp"
#include "services/tracer/Tracer.hpp"
#include "services/util/Flushable.hpp"
#include <cstdarg>

namespace services
{
    class LogAndAbortTracer
    {
        using TracerProvider = infra::Function<services::Tracer&()>;

        services::Tracer* tracer{};
        services::Flushable* flushable{};
        TracerProvider tracerProvider;

    public:
        explicit LogAndAbortTracer(services::Tracer& tracer);
        explicit LogAndAbortTracer(services::Tracer& tracer, services::Flushable& flushable);
        explicit LogAndAbortTracer(TracerProvider tracerProvider);

        ~LogAndAbortTracer();

    private:
        void TraceAbort(services::Tracer& tracer, const char* reason, const char* file, int line, const char* format, va_list* args);
    };
}

#endif // SERVICES_UTIL_LOGANDABORTTRACER_HPP
