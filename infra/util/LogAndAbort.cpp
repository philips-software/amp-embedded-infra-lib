#include "infra/util/LogAndAbort.hpp"
#include <atomic>
#include <cstdarg>

namespace infra
{
    static LogAndAbortHook logAndAbortHook = nullptr;
}

namespace
{
    void logAndAbortHookForwarder(const char* format, ...)
    {
        if (infra::logAndAbortHook)
        {
            va_list args;
            va_start(args, format);
            infra::logAndAbortHook(format, &args);
            va_end(args);
        }
    }
}

namespace infra
{
    void RegisterLogAndAbortHook(LogAndAbortHook hook)
    {
        logAndAbortHook = std::move(hook);
    }

    void ExecuteLogAndAbortHook(const char* reason, const char* file, int line, const char* format, ...)
    {
        static std::atomic<bool> busy{ false };

        if (busy.exchange(true))
            return;

        if (logAndAbortHook)
        {
            logAndAbortHookForwarder("\n%s! [", reason);

            va_list args;
            va_start(args, format);
            logAndAbortHook(format, &args);
            va_end(args);

#if defined(EMIL_ENABLE_LOGGING_FILE_UPON_ABORT) || defined(EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT)
            if (file)
                logAndAbortHookForwarder("] at %s:%d\n", file, line);
#else
            logAndAbortHookForwarder("]\n");
#endif
        }

        busy = false;
    }
}
