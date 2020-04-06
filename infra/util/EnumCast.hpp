#ifndef INFRA_ENUM_CAST
#define INFRA_ENUM_CAST

#include <type_traits>

namespace infra
{
    // Derive from the normal CamelCased function name style of infra to closer
    // match C++'s built-in cast operators.
    template<typename T>
    constexpr auto enum_cast(T t) -> typename std::underlying_type<T>::type
    {
        return static_cast<typename std::underlying_type<T>::type>(t);
    }
}

#endif
