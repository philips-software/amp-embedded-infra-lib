#include "services/tracer/LogAndAbortTracer.hpp"
#include "infra/util/LogAndAbort.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "services/tracer/TracerAdapterPrintf.hpp"
#include <atomic>
#include <cstdarg>
#include <utility>

namespace
{
    static std::atomic<bool> instantiated{};
}

namespace services
{
    LogAndAbortTracer::LogAndAbortTracer(TracerProvider tracerProvider, services::Flushable* flushable)
        : tracerProvider(std::move(tracerProvider))
        , flushable(flushable)
    {
        if (instantiated.exchange(true))
            LOG_AND_ABORT("Only one instance allowed");

        infra::RegisterLogAndAbortHook([this](const char* reason, const char* file, int line, const char* format, va_list* args)
            {
                really_assert(instantiated);
                really_assert(this->tracerProvider);
                auto& tracer = this->tracerProvider();
                TraceAbort(tracer, reason, file, line, format, args);
                if (this->flushable)
                    this->flushable->Flush();
            });
    }

    LogAndAbortTracer::LogAndAbortTracer(TracerProvider tracerProvider)
        : LogAndAbortTracer(std::move(tracerProvider), nullptr)
    {}

    LogAndAbortTracer::LogAndAbortTracer(services::Tracer& tracer)
        : LogAndAbortTracer([&tracer]() -> services::Tracer&
              {
                  return tracer;
              },
              nullptr)
    {}

    LogAndAbortTracer::LogAndAbortTracer(services::Tracer& tracer, services::Flushable& flushable)
        : LogAndAbortTracer([&tracer]() -> services::Tracer&
              {
                  return tracer;
              },
              &flushable)
    {}

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
#ifndef EMIL_HOST_BUILD
        LOG_AND_ABORT("Not destructible");
#endif
        infra::RegisterLogAndAbortHook(nullptr);
        instantiated = false;
    }
}
