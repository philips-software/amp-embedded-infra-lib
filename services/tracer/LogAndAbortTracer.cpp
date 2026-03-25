#include "services/tracer/LogAndAbortTracer.hpp"
#include "infra/util/LogAndAbort.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "services/tracer/TracerAdapterPrintf.hpp"
#include <atomic>
#include <cstdarg>
#include <utility>

namespace
{
    static std::atomic<bool> instansiated{};
}

namespace services
{
    LogAndAbortTracer::LogAndAbortTracer(services::Tracer& tracer)
        : tracer(&tracer)
    {
        really_assert(!instansiated);
        instansiated = true;
        infra::RegisterLogAndAbortHook([this](const char* reason, const char* file, int line, const char* format, va_list* args)
            {
                really_assert(instansiated);
                really_assert(this->tracer);
                TraceAbort(*this->tracer, reason, file, line, format, args);
            });
    }

    LogAndAbortTracer::LogAndAbortTracer(services::Tracer& tracer, services::Flushable& flushable)
        : tracer(&tracer)
        , flushable(&flushable)
    {
        really_assert(!instansiated);
        instansiated = true;
        infra::RegisterLogAndAbortHook([this](const char* reason, const char* file, int line, const char* format, va_list* args)
            {
                really_assert(instansiated);
                really_assert(this->tracer);
                TraceAbort(*this->tracer, reason, file, line, format, args);
                really_assert(this->flushable);
                this->flushable->Flush();
            });
    }

    LogAndAbortTracer::LogAndAbortTracer(TracerProvider tracerProvider)
        : tracerProvider(std::move(tracerProvider))
    {
        really_assert(!instansiated);
        instansiated = true;
        infra::RegisterLogAndAbortHook([this](const char* reason, const char* file, int line, const char* format, va_list* args)
            {
                really_assert(instansiated);
                really_assert(this->tracerProvider);
                auto& tracer = this->tracerProvider();
                TraceAbort(tracer, reason, file, line, format, args);
            });
    }

    void LogAndAbortTracer::TraceAbort(services::Tracer& tracer, const char* reason, const char* file, int line, const char* format, va_list* args)
    {
        tracer.Trace();
        tracer.Trace() << reason << "! [";

        TracerAdapterPrintf{ tracer }.Print(format, args);

        tracer.Continue() << "] ";

        if (file)
            tracer.Continue() << "at " << file << ":" << line;

        tracer.Trace();
    }

    LogAndAbortTracer::~LogAndAbortTracer()
    {
        instansiated = false;
    }
}
