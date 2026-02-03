#include "infra/util/ReallyAssert.hpp"
#include <atomic>

#if INFRA_UTIL_REALLY_ASSERT_LOGGING_ENABLED
namespace infra
{
    static AssertionFailureHandler customHandler = nullptr;

    void RegisterAssertionFailureHandler(AssertionFailureHandler handler)
    {
        customHandler = std::move(handler);
    }

    void HandleAssertionFailure(const char* condition, const char* file, int line)
    {
        static std::atomic<bool> busy{ false };

        if (busy.exchange(true))
            return;

        if (customHandler)
            customHandler(condition, file, line);
        else if constexpr (INFRA_UTIL_LOG_AND_ABORT_ENABLED)
#if defined(EMIL_ENABLE_LOGGING_FILE_UPON_ABORT) || defined(EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT)
            infra::ExecuteLogAndAbortHook("Assertion failed", file, line, "%s", condition);
#else
            infra::ExecuteLogAndAbortHook("Assertion failed", nullptr, 0, "%s", condition);
#endif

        busy = false;
    }
}
#endif
