#ifndef INFRA_AUTO_RESET_FUNCTION_HPP
#define INFRA_AUTO_RESET_FUNCTION_HPP

#include "infra/util/Function.hpp"
#include "infra/util/PostAssign.hpp"

namespace infra
{
    template<class F, std::size_t ExtraSize = INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>
    class AutoResetFunction;

    template<std::size_t ExtraSize, class Result, class... Args>
    class AutoResetFunction<Result(Args...), ExtraSize>
    {
    public:
        typedef Result ResultType;

    public:
        AutoResetFunction() = default;
        explicit AutoResetFunction(std::nullptr_t);
        AutoResetFunction(const AutoResetFunction& other) = delete;
        AutoResetFunction(AutoResetFunction&& other);

        template<class F>
            AutoResetFunction(F f);

        ~AutoResetFunction() = default;

        AutoResetFunction& operator=(const AutoResetFunction& other) = delete;
        AutoResetFunction& operator=(AutoResetFunction&& other);
        AutoResetFunction& operator=(std::nullptr_t);

        explicit operator bool() const;

        ResultType operator()(Args... args);
        ResultType Invoke(const std::tuple<Args...>& args);

        infra::Function<Result(Args...)> Clone() const;
        void Swap(AutoResetFunction& other);

    private:
        Function<Result(Args...), ExtraSize> function;
    };

    template<std::size_t ExtraSize, class Result, class... Args>
        void swap(AutoResetFunction<Result(Args...), ExtraSize>& x, AutoResetFunction<Result(Args...), ExtraSize>& y);

    template<std::size_t ExtraSize, class Result, class... Args>
        bool operator==(const AutoResetFunction<Result(Args...), ExtraSize>& f, std::nullptr_t);
    template<std::size_t ExtraSize, class Result, class... Args>
        bool operator==(std::nullptr_t, const AutoResetFunction<Result(Args...), ExtraSize>& f);
    template<std::size_t ExtraSize, class Result, class... Args>
        bool operator!=(const AutoResetFunction<Result(Args...), ExtraSize>& f, std::nullptr_t);
    template<std::size_t ExtraSize, class Result, class... Args>
        bool operator!=(std::nullptr_t, const AutoResetFunction<Result(Args...), ExtraSize>& f);

    ////    Implementation    ////

    template<std::size_t ExtraSize, class Result, class... Args>
    AutoResetFunction<Result(Args...), ExtraSize>::AutoResetFunction(std::nullptr_t)
    {}

    template<std::size_t ExtraSize, class Result, class... Args>
    AutoResetFunction<Result(Args...), ExtraSize>::AutoResetFunction(AutoResetFunction&& other)
        : function(other.function)
    {
        other.function = nullptr;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    template<class F>
    AutoResetFunction<Result(Args...), ExtraSize>::AutoResetFunction(F f)
        : function(f)
    {}

    template<std::size_t ExtraSize, class Result, class... Args>
    AutoResetFunction<Result(Args...), ExtraSize>& AutoResetFunction<Result(Args...), ExtraSize>::operator=(std::nullptr_t)
    {
        function = nullptr;
        return *this;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    AutoResetFunction<Result(Args...), ExtraSize>& AutoResetFunction<Result(Args...), ExtraSize>::operator=(AutoResetFunction&& other)
    {
        function = other.function;
        other.function = nullptr;
        return *this;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    AutoResetFunction<Result(Args...), ExtraSize>::operator bool() const
    {
        return function != nullptr;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    Result AutoResetFunction<Result(Args...), ExtraSize>::operator()(Args... args)
    {
        return infra::PostAssign(function, nullptr)(std::forward<Args>(args)...);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    Result AutoResetFunction<Result(Args...), ExtraSize>::Invoke(const std::tuple<Args...>& args)
    {
        Function<Result(Args...), ExtraSize> temporary = function;
        function = nullptr;
        return temporary.Invoke(args);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    infra::Function<Result(Args...)> AutoResetFunction<Result(Args...), ExtraSize>::Clone() const
    {
        return function;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    void AutoResetFunction<Result(Args...), ExtraSize>::Swap(AutoResetFunction& other)
    {
        function.Swap(other.function);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    void swap(AutoResetFunction<Result(Args...), ExtraSize>& x, AutoResetFunction<Result(Args...), ExtraSize>& y)
    {
        x.Swap(y);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool operator==(const AutoResetFunction<Result(Args...), ExtraSize>& f, std::nullptr_t)
    {
        return !f;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool operator==(std::nullptr_t, const AutoResetFunction<Result(Args...), ExtraSize>& f)
    {
        return !f;
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool operator!=(const AutoResetFunction<Result(Args...), ExtraSize>& f, std::nullptr_t)
    {
        return !(f == nullptr);
    }

    template<std::size_t ExtraSize, class Result, class... Args>
    bool operator!=(std::nullptr_t, const AutoResetFunction<Result(Args...), ExtraSize>& f)
    {
        return !(f == nullptr);
    }
}

#endif
