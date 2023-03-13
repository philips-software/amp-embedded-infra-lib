#ifndef INFRA_MOCK_CALLBACK_HPP
#define INFRA_MOCK_CALLBACK_HPP

#include "gmock/gmock.h"
#include "infra/util/Function.hpp"

namespace infra
{
    template<class T>
    class MockCallback;

    template<class T>
    class MockCallback<T()>
    {
    public:
        MOCK_CONST_METHOD0_T(callback, T());
    };

    template<class T, class P1>
    class MockCallback<T(P1)>
    {
    public:
        MOCK_CONST_METHOD1_T(callback, T(P1));
    };
    
    template<class T, class P1, class P2>
    class MockCallback<T(P1, P2)>
    {
    public:
        MOCK_CONST_METHOD2_T(callback, T(P1, P2));
    };

    template<class T, class P1, class P2, class P3>
    class MockCallback<T(P1, P2, P3)>
    {
    public:
        MOCK_CONST_METHOD3_T(callback, T(P1, P2, P3));
    };

    template<class T, class P1, class P2, class P3, class P4>
    class MockCallback<T(P1, P2, P3, P4)>
    {
    public:
        MOCK_CONST_METHOD4_T(callback, T(P1, P2, P3, P4));
    };

    template<class T, std::size_t ExtraSize = INFRA_DEFAULT_FUNCTION_EXTRA_SIZE>
    class VerifyingFunctionMock;

    template<class T, std::size_t ExtraSize>
    class VerifyingFunctionMock<T(), ExtraSize>
    {
    public:
        VerifyingFunctionMock()
        {
            EXPECT_CALL(*this, callback());
        }

        MOCK_CONST_METHOD0_T(callback, T());

        operator infra::Function<T(), ExtraSize>()
        {
            return [this]() { return callback(); };
        }
    };

    template<class T, class P1, std::size_t ExtraSize>
    class VerifyingFunctionMock<T(P1), ExtraSize>
    {
    public:
        VerifyingFunctionMock(P1 p1)
        {
            EXPECT_CALL(*this, callback(p1));
        }

        MOCK_CONST_METHOD1_T(callback, T(P1));

        operator infra::Function<T(P1), ExtraSize>()
        {
            return [this](P1 p1) { return callback(p1); };
        }
    };

    template<class T, class P1, class P2, std::size_t ExtraSize>
    class VerifyingFunctionMock<T(P1, P2), ExtraSize>
    {
    public:
        VerifyingFunctionMock(P1 p1, P2 p2)
        {
            EXPECT_CALL(*this, callback(p1, p2));
        }

        MOCK_CONST_METHOD2_T(callback, T(P1, P2));

        operator infra::Function<T(P1, P2), ExtraSize>()
        {
            return [this](P1 p1, P2 p2) { return callback(p1, p2); };
        }
    };

    template<class T, class P1, class P2, class P3, std::size_t ExtraSize>
    class VerifyingFunctionMock<T(P1, P2, P3), ExtraSize>
    {
    public:
        VerifyingFunctionMock(P1 p1, P2 p2, P3 p3)
        {
            EXPECT_CALL(*this, callback(p1, p2, p3));
        }

        MOCK_CONST_METHOD3_T(callback, T(P1, P2, P3));

        operator infra::Function<T(P1, P2, P3), ExtraSize>()
        {
            return [this](P1 p1, P2 p2, P3 p3) { return callback(p1, p2, p3); };
        }
    };
}

#endif
