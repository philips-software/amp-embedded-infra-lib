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
    public:
        using TracerProvider = infra::Function<services::Tracer&()>;

        explicit LogAndAbortTracer(TracerProvider tracerProvider);
        explicit LogAndAbortTracer(services::Tracer& tracer);
        LogAndAbortTracer(services::Tracer& tracer, services::Flushable& flushable);

        ~LogAndAbortTracer();

        LogAndAbortTracer(const LogAndAbortTracer&) = delete;
        LogAndAbortTracer& operator=(const LogAndAbortTracer&) = delete;
        LogAndAbortTracer(LogAndAbortTracer&&) = delete;
        LogAndAbortTracer& operator=(LogAndAbortTracer&&) = delete;

    private:
        LogAndAbortTracer(TracerProvider tracerProvider, services::Flushable* flushable);

        void TraceAbort(services::Tracer& tracer, const char* reason, const char* file, int line, const char* format, va_list* args);

    private:
        services::Flushable* flushable{};
        TracerProvider tracerProvider;
    };
}

#endif // SERVICES_UTIL_LOGANDABORTTRACER_HPP
