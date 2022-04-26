#ifndef INFRA_MULTI_FUNCTION_HPP
#define INFRA_MULTI_FUNCTION_HPP

#include "infra/util/Function.hpp"
#include "infra/util/Variant.hpp"

namespace infra
{
    template<class Fn, class L>
    struct IsInvocableList;

    template<class Fn, class... Args>
    struct IsInvocableList<Fn, List<Args...>>
        : std::is_invocable<Fn, Args...>
    {};

    template<class... F>
    class MultiFunctionHelper
    {
    public:
        template<class FOther>
        using And = MultiFunctionHelper<F..., infra::Function<FOther>>;

        MultiFunctionHelper() = default;

        MultiFunctionHelper(std::nullptr_t)
        {}

        template<class... Args>
        struct IndexOf
        {
            template<std::size_t V, class Enable, class... FOthers>
            struct Helper;

            template<std::size_t V, class F0, class... FOthers>
            struct Helper<V, std::true_type, F0, FOthers...>
            {
                static const std::size_t Value = V;
            };

            template<std::size_t V, class F0, class F1, class... FOthers>
            struct Helper<V, std::false_type, F0, F1, FOthers...>
            {
                static const std::size_t Value = Helper<V + 1, typename std::is_invocable<F1, Args...>::type, F1, FOthers...>::Value;
            };

            static const std::size_t Value = Helper<0, typename std::is_invocable<typename infra::Front<F...>::Type, Args...>::type, F...>::Value;
        };

        template<class F2>
        struct IndexOfFunctionObject
        {
            template<std::size_t V, class Enable, class... FOthers>
            struct Helper;

            template<std::size_t V, class F0, class... FOthers>
            struct Helper<V, std::true_type, F0, FOthers...>
            {
                static const std::size_t Value = V;
            };

            template<std::size_t V, class F0, class F1, class... FOthers>
            struct Helper<V, std::false_type, F0, F1, FOthers...>
            {
                static const std::size_t Value = Helper<V + 1, typename IsInvocableList<F2, typename F1::Arguments>::type, F1, FOthers...>::Value;
            };

            static const std::size_t Value = Helper<0, typename IsInvocableList<F2, typename infra::Front<F...>::Type::Arguments>::type, F...>::Value;
        };

        template<class F2>
        MultiFunctionHelper(F2 f)
        {
            functions.template Emplace<typename infra::TypeAtIndex<IndexOfFunctionObject<F2>::Value, F...>::Type>(f);
        }

        template<class... Args>
        struct ResultOf
        {
            using Type = typename infra::TypeAtIndex<IndexOf<Args...>::Value, F...>::Type::ResultType;
        };

        template<class... Args>
        typename ResultOf<Args...>::Type operator()(Args&&... args)
        {
            return functions.template GetAtIndex<IndexOf<Args...>::Value + 1>()(std::forward<Args>(args)...);
        }

        template<class... Args>
        bool Invocable([[maybe_unused]] Args&&... args)
        {
            return functions.Which() == IndexOf<Args...>::Value + 1;
        }

        explicit operator bool() const
        {
            return functions.Which() != 0;
        }

        bool operator==(const std::nullptr_t) const
        {
            return functions.Which() == 0;
        }

        friend bool operator==(const std::nullptr_t, const MultiFunctionHelper<F...>& f)
        {
            return f.functions.Which() == 0;
        }

        bool operator!=(const std::nullptr_t) const
        {
            return functions.Which() != 0;
        }

        friend bool operator!=(const std::nullptr_t, const MultiFunctionHelper<F...>& f)
        {
            return f.functions.Which() != 0;
        }

    private:
        infra::Variant<infra::None, F...> functions;
    };

    template<class F>
    using MultiFunction = MultiFunctionHelper<infra::Function<F>>;
}

#endif
