#ifndef INFRA_UTIL_LOGANDABORT_HPP
#define INFRA_UTIL_LOGANDABORT_HPP

#include "infra/util/Function.hpp"

namespace infra
{
    using LogAndAbortHook = infra::Function<void(const char* message)>;

    // Note: This hook could be called multiple times before an abort
    void RegisterLogAndAbortHook(LogAndAbortHook hook);

    void HandleLogAndAbort(const char* message);
}

#if defined(EMIL_HOST_BUILD) || defined(EMIL_INFRA_ENABLE_LOG_AND_ABORT)
#define INFRA_UTIL_LOG_AND_ABORT_ENABLED 1
#else
#define INFRA_UTIL_LOG_AND_ABORT_ENABLED 0
#endif

#if INFRA_UTIL_LOG_AND_ABORT_ENABLED
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(msg) \
    infra::HandleLogAndAbort(msg)
#else
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(msg)
#endif

#define log_and_abort(message)                     \
    do                                             \
    {                                              \
        INFRA_UTIL_LOG_AND_ABORT_HANDLER(message); \
        std::abort();                              \
    } while (0)

#endif // INFRA_UTIL_LOGANDABORT_HPP
