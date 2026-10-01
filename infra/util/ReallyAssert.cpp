#include "infra/util/ReallyAssert.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/LogAndAbort.hpp"
#include <atomic>
#include <utility>

#if defined(INFRA_UTIL_REALLY_ASSERT_LOGGING_ENABLED)
namespace infra
{
    namespace
    {
        AssertionFailureHandler customHandler = nullptr;
    }

    void RegisterAssertionFailureHandler(AssertionFailureHandler handler)
    {
        customHandler = std::move(handler);
    }

    void HandleAssertionFailure(const char* condition, const char* file, int line)
    {
        static std::atomic<bool> busy{ false };

        if (busy.exchange(true))
            return;

        infra::ExecuteOnDestruction clearBusy([]
            {
                busy = false;
            });

        if (customHandler)
            customHandler(condition, file, line);
        else if constexpr (INFRA_UTIL_LOG_AND_ABORT_ENABLED)
            infra::ExecuteLogAndAbortHook("Assertion failed", file, line, "%s", condition);
    }
}
#endif // INFRA_UTIL_REALLY_ASSERT_LOGGING_ENABLED
