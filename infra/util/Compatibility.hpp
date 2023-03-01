#ifndef INFRA_COMPATIBILITY_HPP
#define INFRA_COMPATIBILITY_HPP

#include <iterator>
#include <type_traits>

// This file contains compatibility wrappers for features that
// are not present in all versions of the C++ standard.
// It tries to provide a homogeneous way to use these features
// in client-code; while maintaining compatibility across standards
// from C++11 and higher.

#if defined(__cplusplus) && defined(__has_cpp_attribute)
#define EMIL_HAS_CPP_ATTRIBUTE(attribute) __has_cpp_attribute(attribute)
#else
#define EMIL_HAS_CPP_ATTRIBUTE(attribute) (0)
#endif

#ifdef __cpp_if_constexpr
#define IF_CONSTEXPR constexpr
#else
#define IF_CONSTEXPR
#endif

#if __cplusplus > 201402L && EMIL_HAS_CPP_ATTRIBUTE(fallthrough)
#define EMIL_FALLTHROUGH [[fallthrough]]
#elif EMIL_HAS_CPP_ATTRIBUTE(gnu::fallthrough)
#define EMIL_FALLTHROUGH [[gnu::fallthrough]]
#elif __cplusplus && EMIL_HAS_CPP_ATTRIBUTE(clang::fallthrough)
// Workaround for llvm.org/PR23435, since clang 3.6 and below emit a spurious
// error when __has_cpp_attribute is given a scoped attribute in C mode.
#define EMIL_FALLTHROUGH [[clang::fallthrough]]
#else
// gcc recognizes fall through comments as can be seen on:
// https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wimplicit-fallthrough
#define EMIL_FALLTHROUGH /* fall through */
#endif

#if __cplusplus > 201402L && EMIL_HAS_CPP_ATTRIBUTE(maybe_unused)
#define EMIL_MAYBE_UNUSED [[maybe_unused]]
#else
#define EMIL_MAYBE_UNUSED
#endif

namespace infra
{
    static_assert(__cplusplus != 199711L, "The C++ standard selected is too old, or the /Zc:__cplusplus flag has not been set when using MSVC");

    // Compatibility helper for deprecation of std::result_of in C++17
    // See: https://en.cppreference.com/w/cpp/types/result_of
    template<class F, class... ArgTypes>
#if __cplusplus < 201703L
    using result_of_t = typename std::result_of<F(ArgTypes...)>::type;
#else
    using result_of_t = std::invoke_result_t<F, ArgTypes...>;
#endif

    template<bool B, class T = void>
#if __cplusplus < 201402L
    using enable_if_t = typename enable_if<B, T>::type;
#else
    using enable_if_t = std::enable_if_t<B, T>;
#endif

    template<class Iterator>
    std::reverse_iterator<Iterator> make_reverse_iterator(Iterator i)
    {
#ifdef __cpp_lib_make_reverse_iterator
        return std::make_reverse_iterator(i);
#else
        return std::reverse_iterator<Iterator>(i);
#endif
    }
}

#endif
