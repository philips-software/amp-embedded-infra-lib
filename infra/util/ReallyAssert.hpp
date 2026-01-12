#ifndef INFRA_REALLY_ASSERT_HPP
#define INFRA_REALLY_ASSERT_HPP

//  really_assert is a macro similar to assert, however it is not compiled away in release mode

#include "infra/util/Function.hpp"
#include <cassert>
#include <cstdlib>

#if defined(EMIL_HOST_BUILD) || defined(EMIL_ENABLE_REALLY_ASSERT_LOGGING)

#define INFRA_UTIL_REALLY_ASSERT_LOGGING_ENABLED 1
#if defined(EMIL_HOST_BUILD)
// For host builds, always log filenames upon abort
#define EMIL_ENABLE_LOGGING_FILE_UPON_ABORT
#endif

#else
#define INFRA_UTIL_REALLY_ASSERT_LOGGING_ENABLED 0
#endif

#if EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT
#ifndef __FILE_NAME__
// Only available in some compilers, for example GCC 12 or later
#error "__FILE_NAME__ must be available when EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT is set"
#endif
#define INFRA_UTIL_REALLY_ASSERT_TRIGGER_FILE_NAME __FILE_NAME__

#else
#define INFRA_UTIL_REALLY_ASSERT_TRIGGER_FILE_NAME __FILE__
#endif // EMIL_ENABLE_LOGGING_ONLY_FILENAMES_UPON_ABORT

#if INFRA_UTIL_REALLY_ASSERT_LOGGING_ENABLED
namespace infra
{
    using AssertionFailureHandler = infra::Function<void(const char* condition, const char* file, int line)>;

    void RegisterAssertionFailureHandler(AssertionFailureHandler handler);

    void HandleAssertionFailure(const char* condition, const char* file, int line);
}

#define INFRA_UTIL_REALLY_ASSERT_TRIGGER(condition) \
    infra::HandleAssertionFailure(#condition, INFRA_UTIL_REALLY_ASSERT_TRIGGER_FILE_NAME, __LINE__)

#else
#define INFRA_UTIL_REALLY_ASSERT_TRIGGER(condition)
#endif

#ifdef NDEBUG
#define really_assert(condition)                         \
    do                                                   \
    {                                                    \
        if (!(condition))                                \
        {                                                \
            INFRA_UTIL_REALLY_ASSERT_TRIGGER(condition); \
            std::abort();                                \
        }                                                \
    } while (0)
#else
#define really_assert(condition) assert(condition)
#endif

#endif
