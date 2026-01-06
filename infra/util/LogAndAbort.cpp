#include "infra/util/LogAndAbort.hpp"

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
            logAndAbortHook(format);
    }
}
