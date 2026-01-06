#ifndef INFRA_REALLY_ASSERT_HPP
#define INFRA_REALLY_ASSERT_HPP

//  really_assert is a macro similar to assert, however it is not compiled away in release mode

#include <cassert>
#include <cstdlib>
#include <functional>

namespace infra
{
    using AssertionFailureHandler = std::function<void(const char* condition, const char* file, int line)>;

    void RegisterAssertionFailureHandler(AssertionFailureHandler handler);

    void HandleAssertionFailure(const char* condition, const char* file, int line);
}

#if defined(EMIL_HOST_BUILD) || defined(EMIL_ENABLE_LOG_AND_ABORT_LOGGING)
#define INFRA_UTIL_REALLY_ASSERT_TRIGGER(condition) \
    infra::HandleAssertionFailure(#condition, __FILE__, __LINE__)
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
