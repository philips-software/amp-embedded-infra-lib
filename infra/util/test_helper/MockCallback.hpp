#ifndef INFRA_MOCK_CALLBACK_HPP
#define INFRA_MOCK_CALLBACK_HPP

#include "infra/util/Function.hpp"
#include "gmock/gmock.h"
#include <memory>

namespace infra
{
    template<class T>
    class MockCallback;

    template<class T>
    class MockCallback<T()>
    {
    public:
        MOCK_METHOD(T, callback, (), (const));
    };

    template<class T, class P1>
    class MockCallback<T(P1)>
    {
    public:
        MOCK_METHOD(T, callback, (P1), (const));
    };

    template<class T, class P1, class P2>
    class MockCallback<T(P1, P2)>
    {
    public:
        MOCK_METHOD(T, callback, (P1, P2), (const));
    };

    template<class T, class P1, class P2, class P3>
    class MockCallback<T(P1, P2, P3)>
    {
    public:
        MOCK_METHOD(T, callback, (P1, P2, P3), (const));
    };

    template<class T, class P1, class P2, class P3, class P4>
    class MockCallback<T(P1, P2, P3, P4)>
    {
    public:
        MOCK_METHOD(T, callback, (P1, P2, P3, P4), (const));
    };

    template<class T, std::size_t ExtraSize = INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>
    class VerifyingFunction;

    template<class T, std::size_t ExtraSize>
    class VerifyingFunction<T(), ExtraSize>
        : public MockCallback<T()>
    {
    public:
        VerifyingFunction()
        {
            EXPECT_CALL(*this, callback());
        }

        operator infra::Function<T(), ExtraSize>()
        {
            return [this]()
            {
                return MockCallback<T()>::callback();
            };
        }
    };

    template<class T, class P1, std::size_t ExtraSize>
    class VerifyingFunction<T(P1), ExtraSize>
        : public MockCallback<T(P1)>
    {
    public:
        VerifyingFunction(P1 p1)
        {
            EXPECT_CALL(*this, callback(p1));
        }

        operator infra::Function<T(P1), ExtraSize>()
        {
            return [this](P1 p1)
            {
                return MockCallback<T(P1)>::callback(p1);
            };
        }
    };

    template<class T, class P1, class P2, std::size_t ExtraSize>
    class VerifyingFunction<T(P1, P2), ExtraSize>
        : public MockCallback<T(P1, P2)>
    {
    public:
        VerifyingFunction(P1 p1, P2 p2)
        {
            EXPECT_CALL(*this, callback(p1, p2));
        }

        operator infra::Function<T(P1, P2), ExtraSize>()
        {
            return [this](P1 p1, P2 p2)
            {
                return MockCallback<T(P1, P2)>::callback(p1, p2);
            };
        }
    };

    template<class T, class P1, class P2, class P3, std::size_t ExtraSize>
    class VerifyingFunction<T(P1, P2, P3), ExtraSize>
        : public MockCallback<T(P1, P2, P3)>
    {
    public:
        VerifyingFunction(P1 p1, P2 p2, P3 p3)
        {
            EXPECT_CALL(*this, callback(p1, p2, p3));
        }

        operator infra::Function<T(P1, P2, P3), ExtraSize>()
        {
            return [this](P1 p1, P2 p2, P3 p3)
            {
                return MockCallback<T(P1, P2, P3)>::callback(p1, p2, p3);
            };
        }
    };

    template<class Result>
    class VerifyingInvoker;

    template<class Result, class... Args>
    class VerifyingInvoker<Result(Args...)>
    {
    public:
        explicit VerifyingInvoker(infra::Function<Result(Args...)> onCalled)
            : onCalled(onCalled)
        {
            EXPECT_CALL(*this, callback());
        }

        MOCK_CONST_METHOD0_T(callback, Result());

        operator infra::Function<Result(Args...)>()
        {
            return [this](Args&&... args)
            {
                onCalled(args...);
                return callback();
            };
        }

    private:
        infra::Function<Result(Args...)> onCalled;
    };

    template<class R, std::size_t ExtraSize = INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>
    auto MockFunction = []
    {
        std::abort();
    };

    template<std::size_t ExtraSize, class Result, class... Args>
    constexpr auto MockFunction<Result(Args...), ExtraSize> = [](auto&&... matchers) -> Function<Result(Args...), ExtraSize>
    {
        auto ptr = std::make_shared<testing::MockFunction<Result(Args...)>>();

        EXPECT_CALL(*ptr, Call(std::forward<decltype(matchers)>(matchers)...));

        return [ptr](Args&&... args) -> Result
        {
            return ptr->Call(std::forward<Args>(args)...);
        };
    };
}

#endif
