#include "infra/util/LogAndAbort.hpp"
#include <cstdarg>

namespace infra
{
    static LogAndAbortHook logAndAbortHook = nullptr;

    void logAndAbortHookForwarder(const char* format, ...)
    {
        if (logAndAbortHook)
        {
            va_list args;
            va_start(args, format);
            logAndAbortHook(format, &args);
            va_end(args);
        }
    }

    void RegisterLogAndAbortHook(LogAndAbortHook hook)
    {
        logAndAbortHook = std::move(hook);
    }

    void ExecuteLogAndAbortHook(const char* file, int line, const char* format, ...)
    {
        if (logAndAbortHook)
        {
#if defined(EMIL_ENABLE_LOGGING_FILE_UPON_ABORT) || defined(EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT)
            if (file)
                logAndAbortHookForwarder("\n%s:%d ", file, line);
#endif

            va_list args;
            va_start(args, format);
            logAndAbortHook(format, &args);
            va_end(args);
        }
    }
}
