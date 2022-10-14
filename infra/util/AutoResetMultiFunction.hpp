#ifndef INFRA_AUTO_RESET_MULTI_FUNCTION_HPP
#define INFRA_AUTO_RESET_MULTI_FUNCTION_HPP

#include "infra/util/MultiFunction.hpp"
#include "infra/util/PostAssign.hpp"

namespace infra
{
    template<class... F>
    class AutoResetMultiFunctionHelper
    {
    public:
        template<class FOther>
        using And = AutoResetMultiFunctionHelper<F..., infra::Function<FOther>>;

        AutoResetMultiFunctionHelper() = default;
        explicit AutoResetMultiFunctionHelper(std::nullptr_t);
        AutoResetMultiFunctionHelper(const AutoResetMultiFunctionHelper& other) = delete;
        AutoResetMultiFunctionHelper(AutoResetMultiFunctionHelper&& other);
        template<class F1>
        AutoResetMultiFunctionHelper(F1&& f);

        AutoResetMultiFunctionHelper& operator=(const AutoResetMultiFunctionHelper& other) = delete;
        AutoResetMultiFunctionHelper& operator=(AutoResetMultiFunctionHelper&& other);
        AutoResetMultiFunctionHelper& operator=(std::nullptr_t);

        template<class... Args>
        typename MultiFunctionHelper<F...>::template ResultOf<Args...>::Type operator()(Args&&... args);

        explicit operator bool() const;

        MultiFunctionHelper<F...> Clone() const;

    private:
        MultiFunctionHelper<F...> multiFunction;
    };

    template<class... F>
    bool operator==(const AutoResetMultiFunctionHelper<F...>& f, std::nullptr_t);
    template<class... F>
    bool operator==(std::nullptr_t, const AutoResetMultiFunctionHelper<F...>& f);
    template<class... F>
    bool operator!=(const AutoResetMultiFunctionHelper<F...>& f, std::nullptr_t);
    template<class... F>
    bool operator!=(std::nullptr_t, const AutoResetMultiFunctionHelper<F...>& f);

    template<class F>
    using AutoResetMultiFunction = AutoResetMultiFunctionHelper<infra::Function<F>>;

    ////    Implementation    ////

    template<class... F>
    AutoResetMultiFunctionHelper<F...>::AutoResetMultiFunctionHelper(std::nullptr_t)
    {}

    template<class... F>
    AutoResetMultiFunctionHelper<F...>::AutoResetMultiFunctionHelper(AutoResetMultiFunctionHelper&& other)
        : multiFunction(other.multiFunction)
    {
        other.multiFunction = nullptr;
    }

    template<class... F>
    template<class F1>
    AutoResetMultiFunctionHelper<F...>::AutoResetMultiFunctionHelper(F1&& f)
        : multiFunction(std::forward<F1>(f))
    {}

    template<class... F>
    AutoResetMultiFunctionHelper<F...>& AutoResetMultiFunctionHelper<F...>::operator=(std::nullptr_t)
    {
        multiFunction = nullptr;
        return *this;
    }

    template<class... F>
    AutoResetMultiFunctionHelper<F...>& AutoResetMultiFunctionHelper<F...>::operator=(AutoResetMultiFunctionHelper&& other)
    {
        multiFunction = other.multiFunction;
        other.multiFunction = nullptr;
        return *this;
    }

    template<class... F>
    template<class... Args>
    typename MultiFunctionHelper<F...>::template ResultOf<Args...>::Type AutoResetMultiFunctionHelper<F...>::operator()(Args&&... args)
    {
        return infra::PostAssign(multiFunction, nullptr)(std::forward<Args>(args)...);
    }

    template<class... F>
    AutoResetMultiFunctionHelper<F...>::operator bool() const
    {
        return multiFunction != nullptr;
    }

    template<class... F>
    infra::MultiFunctionHelper<F...> AutoResetMultiFunctionHelper<F...>::Clone() const
    {
        return multiFunction;
    }

    template<class... F>
    bool operator==(const AutoResetMultiFunctionHelper<F...>& f, std::nullptr_t)
    {
        return !f;
    }

    template<class... F>
    bool operator==(std::nullptr_t, const AutoResetMultiFunctionHelper<F...>& f)
    {
        return !f;
    }

    template<class... F>
    bool operator!=(const AutoResetMultiFunctionHelper<F...>& f, std::nullptr_t)
    {
        return !(f == nullptr);
    }

    template<class... F>
    bool operator!=(std::nullptr_t, const AutoResetMultiFunctionHelper<F...>& f)
    {
        return !(f == nullptr);
    }
}

#endif
