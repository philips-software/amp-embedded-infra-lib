#include "services/tracer/LogAndAbortTracer.hpp"
#include "infra/util/LogAndAbort.hpp"
#include "services/tracer/TracerAdapterPrintf.hpp"
#include <cstdarg>
#include <utility>

namespace services
{
    namespace
    {
        LogAndAbortTracerProvider logAndAbortTracerProvider = nullptr;

        void SetHook()
        {
            infra::RegisterLogAndAbortHook([](const char* reason, const char* file, int line, const char* format, va_list* args)
                {
                    if (logAndAbortTracerProvider)
                    {
                        auto& tracer = logAndAbortTracerProvider();

                        tracer.Trace();
                        tracer.Trace() << reason << "! [";

                        TracerAdapterPrintf{ tracer }.Print(format, args);

                        tracer.Continue() << "] ";

                        if (file)
                            tracer.Continue() << "at " << file << ":" << line;

                        tracer.Trace();
                    }
                });
        }

        void ClearHook()
        {
            infra::RegisterLogAndAbortHook(nullptr);
        }
    }

    void RegisterLogAndAbortTracerProvider(LogAndAbortTracerProvider tracerProvider)
    {
        logAndAbortTracerProvider = std::move(tracerProvider);

        if (logAndAbortTracerProvider == nullptr)
            ClearHook();
        else
            SetHook();
    }
}
