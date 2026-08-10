#include "infra/util/LogAndAbort.hpp"
#include "infra/util/Function.hpp"
#include <atomic>
#include <cstdarg>
#include <utility>

namespace infra
{
    namespace
    {
        LogAndAbortHook& LogAndAbortHookStorage()
        {
            static LogAndAbortHook hook = nullptr;
            return hook;
        }
    }

    void RegisterLogAndAbortHook(LogAndAbortHook hook)
    {
        LogAndAbortHookStorage() = std::move(hook);
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

        auto& hook = LogAndAbortHookStorage();
        if (hook)
        {
            va_list args;
            va_start(args, format);
            hook(reason, file, line, format, &args);
            va_end(args);
        }
    }
}
