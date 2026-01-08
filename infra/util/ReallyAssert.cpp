#include "infra/util/ReallyAssert.hpp"
#include "infra/util/LogAndAbort.hpp"

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
        if (customHandler)
            customHandler(condition, file, line);
        else if constexpr (INFRA_UTIL_LOG_AND_ABORT_ENABLED)
#if defined(EMIL_ENABLE_LOGGING_FILE_UPON_ABORT) || defined(EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT)
            infra::ExecuteLogAndAbortHook(nullptr, 0, "\nAssertion failed [%s] at %s:%d\n", condition, file, line);
#else
            infra::ExecuteLogAndAbortHook(nullptr, 0, "\nAssertion failed [%s]\n", condition);
#endif
    }
}
#endif
