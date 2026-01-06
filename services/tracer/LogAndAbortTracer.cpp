#include "services/tracer/LogAndAbortTracer.hpp"
#include "services/tracer/TracerAdapterPrintf.hpp"

namespace services
{
    static LogAndAbortTracerProvider logAndAbortTracerProvider = nullptr;

    void RegisterLogAndAbortTracerProvider(LogAndAbortTracerProvider tracerProvider)
    {
        logAndAbortTracerProvider = std::move(tracerProvider);

        infra::RegisterLogAndAbortHook([](const char* format, va_list* args)
            {
                if (logAndAbortTracerProvider)
                {
                    TracerAdapterPrintf adapter(logAndAbortTracerProvider());
                    adapter.Print(format, args);
                }
            });
    }
}
