#ifndef INFRA_MOCK_HELPERS_HPP
#define INFRA_MOCK_HELPERS_HPP

#include "gmock/gmock.h"
#include <functional>
#include <type_traits>
#include <vector>

namespace infra
{
    MATCHER_P(CheckByteRangeContents, contents, "")
    {
        return std::vector<uint8_t>(arg.begin(), arg.end()) == std::vector<uint8_t>(contents.begin(), contents.end());
    }

    ACTION_TEMPLATE(SaveRef,
        HAS_1_TEMPLATE_PARAMS(int, k),
        AND_1_VALUE_PARAMS(pointer)) {
        *pointer = &::std::get<k>(args);
    }

    namespace detail
    {
        template<int...>
        struct Sequence
        {};

        template<int N, int... S>
        struct GenerateSequence
            : GenerateSequence<N - 1, N - 1, S...>
        {};

        template<int... S>
        struct GenerateSequence<0, S...>
        {
            typedef Sequence<S...> Type;
        };

        template<class ResultType, class Arguments>
        class LambdaAction;

        template<class ResultType, class... Arguments>
        class LambdaAction<ResultType, std::tuple<Arguments...>>
        {
        public:
            LambdaAction(const std::function<ResultType(Arguments...)>& lambda)
                : lambda(lambda)
            {}

            LambdaAction& operator=(const LambdaAction& other) = delete;

            template<class F>
            operator testing::Action<F>() const
            {
                return testing::Action<F>(new Impl<F>(lambda));
            }

        private:
            template<class F>
            class Impl
                : public testing::ActionInterface<F>
            {
            public:
                typedef typename testing::internal::Function<F>::Result Result;
                typedef typename testing::internal::Function<F>::ArgumentTuple ArgumentTuple;
                typedef typename testing::internal::Function<F>::MakeResultVoid VoidResult;

                Impl(const std::function<ResultType(Arguments...)>& lambda)
                    : lambda(lambda)
                {}

                Impl(const Impl& other) = delete;
                Impl& operator=(const Impl& other) = delete;

                virtual Result Perform(const ArgumentTuple& args)
                {
                    return CallLambda(args, typename GenerateSequence<std::tuple_size<ArgumentTuple>::value>::Type());
                }

                template<int... S>
                Result CallLambda(const ArgumentTuple& args, Sequence<S...>)
                {
                    return lambda(std::get<S>(args)...);
                }

            private:
                std::function<ResultType(Arguments...)> lambda;
            };

        private:
            std::function<ResultType(Arguments...)> lambda;
        };

        template<class T>
        struct LambdaType
            : LambdaType<decltype(&T::operator())>
        {};

        template<class ClassType, class R, class... Args>
        struct LambdaType<R(ClassType::*)(Args...) const>
        {
            using ReturnType = R;
            using ArgumentTuple = std::tuple<Args...>;
        };
    }

    template<class L>
    detail::LambdaAction<typename detail::LambdaType<L>::ReturnType, typename detail::LambdaType<L>::ArgumentTuple> Lambda(L lambda)
    {
        return detail::LambdaAction<typename detail::LambdaType<L>::ReturnType, typename detail::LambdaType<L>::ArgumentTuple>(lambda);
    }
}

#endif
