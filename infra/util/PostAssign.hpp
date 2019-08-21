#ifndef INFRA_POST_ASSIGN_HPP
#define INFRA_POST_ASSIGN_HPP

#include <type_traits>

namespace infra
{
    template<class T>
    T PostAssign(T& value, const typename std::decay<T>::type& newValue)
    {
        T result(value);
        value = newValue;
        return result;
    }
}

#endif
