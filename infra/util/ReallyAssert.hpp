#ifndef INFRA_REALLY_ASSERT_HPP
#define INFRA_REALLY_ASSERT_HPP

//  really_assert is a macro similar to assert, however it is not compiled away in release mode

#include <cassert>
#include <cstdint>
#include <cstdlib>

#ifdef EMIL_HOST_BUILD
#include <functional>

namespace
{
    // Source - https://stackoverflow.com/a/19004720
    // Posted by Chetan Reddy, modified by community. See post 'Timeline' for change history
    // Retrieved 2026-01-06, License - CC BY-SA 4.0

    constexpr int32_t basename_index(const char* const path, const int32_t index = 0, const int32_t slash_index = -1)
    {
        return path[index]
                   ? (path[index] == '/'
                             ? basename_index(path, index + 1, index)
                             : basename_index(path, index + 1, slash_index))
                   : (slash_index + 1);
    }

}

namespace infra
{
    using AssertionFailureHandler = std::function<void(const char* condition, const char* file, int line)>;

    void RegisterAssertionFailureHandler(AssertionFailureHandler handler);

    void HandleAssertionFailure(const char* condition, const char* file, int line);
}
#endif

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

// TODO(HW): Does this work for ehader files and everything?
#define __FILENAME__ ({ static const int32_t basename_idx = basename_index(__FILE__); \
                        static_assert (basename_idx >= 0, "compile-time basename");   \
                        __FILE__ ":" STRINGIZE(__LINE__) ": " + basename_idx; })

#ifdef EMIL_HOST_BUILD
#define INFRA_UTIL_REALLY_ASSERT_TRIGGER(condition) \
    infra::HandleAssertionFailure(#condition, __FILENAME__, __LINE__)
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
