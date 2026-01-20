#ifndef INFRA_UTIL_LOGANDABORT_HPP
#define INFRA_UTIL_LOGANDABORT_HPP

#include "infra/util/Function.hpp"
#include <cstdarg>

namespace infra
{
    using LogAndAbortHook = infra::Function<void(const char* format, va_list* args)>;
    // Note: This hook may be called multiple times per abort.
    void RegisterLogAndAbortHook(LogAndAbortHook hook);
    void ExecuteLogAndAbortHook(const char* reason, const char* file, int line, const char* format, ...);
}

#if defined(EMIL_HOST_BUILD) || defined(EMIL_ENABLE_LOG_AND_ABORT_LOGGING)
#define INFRA_UTIL_LOG_AND_ABORT_ENABLED 1
#if defined(EMIL_HOST_BUILD)
// For host builds, always log filenames upon abort
#define EMIL_ENABLE_LOGGING_FILE_UPON_ABORT
#endif

#else
#define INFRA_UTIL_LOG_AND_ABORT_ENABLED 0
#endif

#if INFRA_UTIL_LOG_AND_ABORT_ENABLED

#if defined(EMIL_ENABLE_LOGGING_FILE_UPON_ABORT) || defined(EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT)
#if EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT
#ifndef __FILE_NAME__
// Only available in some compilers, for example GCC 12 or later
#error "__FILE_NAME__ must be available when EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT is set"
#endif
#define INFRA_UTIL_LOG_AND_ABORT_HOOK_FILE_NAME __FILE_NAME__
#else
#define INFRA_UTIL_LOG_AND_ABORT_HOOK_FILE_NAME __FILE__
#endif // EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT

#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ...) \
    infra::ExecuteLogAndAbortHook(                    \
        "Aborting",                                   \
        INFRA_UTIL_LOG_AND_ABORT_HOOK_FILE_NAME,      \
        __LINE__,                                     \
        format,                                       \
        ##__VA_ARGS__)

#else
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ...) \
    infra::ExecuteLogAndAbortHook("Aborting", nullptr, 0, format, ##__VA_ARGS__)
#endif // defined(EMIL_ENABLE_LOGGING_FILE_UPON_ABORT) || defined(EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT)

#else
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ...)
#endif // INFRA_UTIL_LOG_AND_ABORT_ENABLED

#define LOG_AND_ABORT(format, ...)                               \
    do                                                           \
    {                                                            \
        INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ##__VA_ARGS__); \
        std::abort();                                            \
    } while (0)

#endif // INFRA_UTIL_LOGANDABORT_HPP
