#include "infra/util/ReallyAssert.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace
{
    class Listener
    {
    public:
        MOCK_METHOD(void, OnAssertionFailure, (const char* condition, const char* file, int line), ());
    };

    class ReallyAssertTest
        : public testing::Test
    {
    public:
        ~ReallyAssertTest()
        {
            infra::RegisterAssertionFailureHandler(nullptr);
        }

        testing::MockFunction<void(const char* condition, const char* file, int line)> assertionFailureHandlerMock;
    };
}

TEST_F(ReallyAssertTest, assert_passed_no_abort)
{
    really_assert(true);
}

TEST_F(ReallyAssertTest, assert_failed_abort)
{
    EXPECT_DEATH(really_assert(false), "");
}

TEST_F(ReallyAssertTest, assert_failed_without_handler_does_nothing)
{
    // Manually calling handler because a debug build will call the standard assert instead of really_assert
    infra::HandleAssertionFailure("condition", "file", 42);
}

TEST_F(ReallyAssertTest, assert_failed_with_handler_calls_handler)
{
    infra::RegisterAssertionFailureHandler([this](auto&&... args)
        {
            assertionFailureHandlerMock.Call(std::forward<decltype(args)>(args)...);
        });

    EXPECT_CALL(assertionFailureHandlerMock, Call("condition", "file", 42));

    // Manually calling handler because a debug build will call the standard assert instead of really_assert
    infra::HandleAssertionFailure("condition", "file", 42);
}

TEST_F(ReallyAssertTest, assert_failed_with_message_aborts)
{
    EXPECT_DEATH(really_assert_with_msg(false, "%s", "foo"), "");
}

TEST_F(ReallyAssertTest, assert_passed_with_message_aborts)
{
    really_assert_with_msg(true, "%s", "foo");
}

TEST_F(ReallyAssertTest, assert_failed_recursive_call_skipped)
{
    infra::RegisterAssertionFailureHandler([this](auto&&... args)
        {
            assertionFailureHandlerMock.Call(std::forward<decltype(args)>(args)...);
            infra::HandleAssertionFailure("recursive", "file", 42);
        });

    EXPECT_CALL(assertionFailureHandlerMock, Call("fail", "file", 42));

    // Manually calling handler because a debug build will call the standard assert instead of really_assert
    infra::HandleAssertionFailure("fail", "file", 42);
}
