#ifndef INFRA_UTIL_LOGANDABORT_HPP
#define INFRA_UTIL_LOGANDABORT_HPP

#include "infra/util/Function.hpp"
#include <cstdarg>

namespace infra
{
    using LogAndAbortHook = infra::Function<void(const char* format, va_list* args)>;
    void RegisterLogAndAbortHook(LogAndAbortHook hook);
    void ExecuteLogAndAbortHook(const char* format, ...);
}

#if defined(EMIL_HOST_BUILD) || defined(EMIL_ENABLE_LOG_AND_ABORT_LOGGING)
#define INFRA_UTIL_LOG_AND_ABORT_ENABLED 1
#else
#define INFRA_UTIL_LOG_AND_ABORT_ENABLED 0
#endif

#if INFRA_UTIL_LOG_AND_ABORT_ENABLED
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ...) \
    infra::ExecuteLogAndAbortHook("\n" format "\n", ##__VA_ARGS__)
#else
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ...)
#endif

#define LOG_AND_ABORT(format, ...)                               \
    do                                                           \
    {                                                            \
        INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ##__VA_ARGS__); \
        std::abort();                                            \
    } while (0)

#endif // INFRA_UTIL_LOGANDABORT_HPP
