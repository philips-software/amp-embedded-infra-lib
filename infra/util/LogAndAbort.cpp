#include "infra/util/LogAndAbort.hpp"
#include "infra/util/Function.hpp"
#include <atomic>
#include <cstdarg>
#include <utility>

namespace infra
{
    namespace
    {
        LogAndAbortHook logAndAbortHook = nullptr;
    }

    void RegisterLogAndAbortHook(LogAndAbortHook hook)
    {
        logAndAbortHook = std::move(hook);
    }

    void ExecuteLogAndAbortHook(const char* reason, const char* file, int line, const char* format, ...)
    {
        static std::atomic<bool> busy{ false };

        if (busy.exchange(true))
            return;

        infra::ExecuteOnDestruction clearBusy([]
            {
                busy = false;
            });

        if (logAndAbortHook)
        {
            va_list args;
            va_start(args, format);
            logAndAbortHook(reason, file, line, format, &args);
            va_end(args);
        }
    }
}
