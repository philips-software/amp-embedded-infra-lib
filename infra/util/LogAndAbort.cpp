#include "infra/util/ReallyAssert.hpp"

#ifdef INFRA_UTIL_LOG_AND_ABORT_ENABLED
namespace infra
{
    static LogAndAbortHook logAndAbortHook = nullptr;

    void RegisterLogAndAbortHook(LogAndAbortHook hook)
    {
        logAndAbortHook = std::move(hook);
    }

    void HandleLogAndAbort(const char* message)
    {
        if (logAndAbortHook)
            logAndAbortHook(message);
    }
}
#endif
