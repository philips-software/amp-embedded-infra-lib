#ifndef INFRA_COMPATIBILITY_HPP
#define INFRA_COMPATIBILITY_HPP

#include <type_traits>

namespace infra
{
    // Compatibility helper for deprecation of std::result_of in C++17
    // See: https://en.cppreference.com/w/cpp/types/result_of
    template<class A, class B>
#if __cplusplus < 201703L
    using result_of_t = typename std::result_of<A(B&)>::type;
#else
    using result_of_t = std::invoke_result_t<A, B&>;
#endif
}

#endif
