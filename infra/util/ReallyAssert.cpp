#include "infra/util/ReallyAssert.hpp"
#include "infra/util/LogAndAbort.hpp"

#ifdef EMIL_HOST_BUILD
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
            infra::ExecuteLogAndAbortHook("Assertion failed [%s] at %s:%d\n", condition, file, line);
    }
}
#endif
