#ifndef INFRA_UTIL_LOGANDABORT_HPP
#define INFRA_UTIL_LOGANDABORT_HPP

#include "infra/util/Function.hpp"
#include <cstdarg>
#include <cstdint>
#include <type_traits>

namespace infra
{
    using LogAndAbortHook = infra::Function<void(const char* reason, const char* file, int line, const char* format, va_list* args)>;
    // Note: This hook is called exactly once per LOG_AND_ABORT invocation.
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
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER_HOOK(msg, file, line, format, ...) \
    infra::ExecuteLogAndAbortHook(msg, file, line, format, ##__VA_ARGS__)

#define INFRA_UTIL_LOG_AND_ABORT_HANDLER_MSG(file, line, format, ...) \
    INFRA_UTIL_LOG_AND_ABORT_HANDLER_HOOK("Aborting", file, line, format, ##__VA_ARGS__)

#if defined(EMIL_ENABLE_LOGGING_FILE_UPON_ABORT) || defined(EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT)
#if EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT
#ifndef __FILE_NAME__
// Only available in some compilers, for example GCC 12 or later
#error "__FILE_NAME__ must be available when EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT is set"
#endif
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ...) INFRA_UTIL_LOG_AND_ABORT_HANDLER_MSG(__FILE_NAME__, __LINE__, format, ##__VA_ARGS__)
#else
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ...) INFRA_UTIL_LOG_AND_ABORT_HANDLER_MSG(__FILE__, __LINE__, format, ##__VA_ARGS__)
#endif // EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT
#else
#define INFRA_UTIL_LOG_AND_ABORT_HANDLER(format, ...) INFRA_UTIL_LOG_AND_ABORT_HANDLER_MSG(nullptr, 0, format, ##__VA_ARGS__)
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

#define LOG_AND_ABORT_NOT_IMPLEMENTED() LOG_AND_ABORT("Not implemented")
#define LOG_AND_ABORT_ENUM(value)                                                                                            \
    do                                                                                                                       \
    {                                                                                                                        \
        static_assert(std::is_enum_v<std::decay_t<decltype(value)>>, "LOG_AND_ABORT_ENUM can only be used with enum types"); \
        using UnderlyingType = std::underlying_type_t<std::decay_t<decltype(value)>>;                                        \
        LOG_AND_ABORT("Unexpected enum: %lld", static_cast<int64_t>(static_cast<UnderlyingType>(value)));                    \
    } while (0)

#endif // INFRA_UTIL_LOGANDABORT_HPP
