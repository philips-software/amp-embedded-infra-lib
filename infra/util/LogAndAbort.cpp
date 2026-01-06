#include "infra/util/LogAndAbort.hpp"
#include <cstdarg>

namespace infra
{
    static LogAndAbortHook logAndAbortHook = nullptr;

    void RegisterLogAndAbortHook(LogAndAbortHook hook)
    {
        logAndAbortHook = std::move(hook);
    }

    void HandleLogAndAbort(const char* format, ...)
    {
        if (logAndAbortHook)
        {
            va_list args;
            va_start(args, format);
            logAndAbortHook(format, &args);
            va_end(args);
        }
    }
}
