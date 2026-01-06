#include "infra/util/ReallyAssert.hpp"
#include "infra/util/LogAndAbort.hpp"

#ifdef EMIL_HOST_BUILD
namespace infra
{
    static AssertionFailureHandler customHandler = nullptr;

    void RegisterAssertionFailureHandler(AssertionFailureHandler handler)
    {
        if (INFRA_UTIL_LOG_AND_ABORT_ENABLED)
            LOG_AND_ABORT("Assertion handler not supported when LogAndAbort is enabled");
        customHandler = std::move(handler);
    }

    void HandleAssertionFailure(const char* condition, const char* file, int line)
    {
        if constexpr (INFRA_UTIL_LOG_AND_ABORT_ENABLED)
        {
            // infra::HandleLogAndAbort("Assertion failed [");
            // infra::HandleLogAndAbort(condition);
            // infra::HandleLogAndAbort("] at ");
            // infra::HandleLogAndAbort(file);
            // infra::HandleLogAndAbort(":");
            // infra::HandleLogAndAbort(std::to_string(line).c_str());
        }
        else if (customHandler)
            customHandler(condition, file, line);
    }
}
#endif
