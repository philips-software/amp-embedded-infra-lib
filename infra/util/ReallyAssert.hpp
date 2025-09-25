#ifndef INFRA_REALLY_ASSERT_HPP
#define INFRA_REALLY_ASSERT_HPP

//  really_assert is a macro similar to assert, however it is not compiled away in release mode

#include <cassert>
#include <cstdlib>

#ifdef EMIL_HOST_BUILD
#include <stdexcept>
#include <string>

namespace infra
{
    [[noreturn]] inline void HandleAssertionFailure(const char* condition, const char* file, int line)
    {
        throw std::runtime_error(std::string("Assertion failed: ") + condition +
                                 " at " + file + ":" + std::to_string(line));
    }
}
#endif

#ifdef NDEBUG
#ifdef EMIL_HOST_BUILD
#define really_assert(condition)                                       \
    if (!(condition))                                                  \
    {                                                                  \
        infra::HandleAssertionFailure(#condition, __FILE__, __LINE__); \
    }                                                                  \
    else                                                               \
        for (; false;)
#else
#define really_assert(condition) \
    if (!(condition))            \
    {                            \
        std::abort();            \
    }                            \
    else                         \
        for (; false;)
#endif
#else
#ifdef EMIL_HOST_BUILD
#define really_assert(condition)                                       \
    if (!(condition))                                                  \
    {                                                                  \
        infra::HandleAssertionFailure(#condition, __FILE__, __LINE__); \
    }                                                                  \
    else                                                               \
        for (; false;)
#else
#define really_assert(condition) assert(condition)
#endif
#endif

#endif
