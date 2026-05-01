#ifndef INFRA_UTIL_LOGANDABORT_HPP
#define INFRA_UTIL_LOGANDABORT_HPP

#include "infra/util/Function.hpp"
#include <cstdarg>
#include <cstdlib>
#include <type_traits>

namespace
{
    template<class T, bool = std::is_enum_v<T>>
    struct LogAndAbortEnumOrIntegralType
    {
        using Type = T;
    };

    template<class T>
    struct LogAndAbortEnumOrIntegralType<T, true>
    {
        using Type = std::underlying_type_t<T>;
    };

    template<class T>
    using LogAndAbortEnumOrIntegralTypeT = typename LogAndAbortEnumOrIntegralType<T>::Type;
}

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
#define LOG_AND_ABORT_ENUM(value)                                                                                                                                                             \
    do                                                                                                                                                                                        \
    {                                                                                                                                                                                         \
        static_assert(std::is_enum_v<std::decay_t<decltype(value)>> || std::is_integral_v<std::decay_t<decltype(value)>>, "LOG_AND_ABORT_ENUM can only be used with enum or integral types"); \
        using LogAndAbortValueType = LogAndAbortEnumOrIntegralTypeT<std::decay_t<decltype(value)>>;                                                                                           \
        if constexpr (std::is_enum_v<std::decay_t<decltype(value)>>)                                                                                                                          \
        {                                                                                                                                                                                     \
            if constexpr (std::is_signed_v<LogAndAbortValueType>)                                                                                                                             \
            {                                                                                                                                                                                 \
                LOG_AND_ABORT("Unexpected enum: %lld", static_cast<long long>(static_cast<LogAndAbortValueType>(value)));                                                                     \
            }                                                                                                                                                                                 \
            else                                                                                                                                                                              \
            {                                                                                                                                                                                 \
                LOG_AND_ABORT("Unexpected enum: %llu", static_cast<unsigned long long>(static_cast<LogAndAbortValueType>(value)));                                                            \
            }                                                                                                                                                                                 \
        }                                                                                                                                                                                     \
        else if constexpr (std::is_integral_v<std::decay_t<decltype(value)>>)                                                                                                                 \
        {                                                                                                                                                                                     \
            if constexpr (std::is_signed_v<LogAndAbortValueType>)                                                                                                                             \
            {                                                                                                                                                                                 \
                LOG_AND_ABORT("Unexpected integral: %lld", static_cast<long long>(value));                                                                                                    \
            }                                                                                                                                                                                 \
            else                                                                                                                                                                              \
            {                                                                                                                                                                                 \
                LOG_AND_ABORT("Unexpected integral: %llu", static_cast<unsigned long long>(value));                                                                                           \
            }                                                                                                                                                                                 \
        }                                                                                                                                                                                     \
    } while (0)

#endif // INFRA_UTIL_LOGANDABORT_HPP
