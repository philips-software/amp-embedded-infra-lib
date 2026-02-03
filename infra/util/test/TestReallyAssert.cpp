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
        Listener listener;
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
    infra::RegisterAssertionFailureHandler([&](auto condition, auto file, auto line)
        {
            listener.OnAssertionFailure(condition, file, line);
        });
    EXPECT_CALL(listener, OnAssertionFailure(testing::_, testing::_, testing::_)).Times(1);

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
    infra::RegisterAssertionFailureHandler([&](auto condition, auto, auto)
        {
            std::cout << condition << std::endl;
            infra::HandleAssertionFailure("recursive", "file", 42);
        });

    testing::internal::CaptureStdout();
    // Manually calling handler because a debug build will call the standard assert instead of really_assert
    infra::HandleAssertionFailure("fail", "file", 42);

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_THAT(output, testing::HasSubstr("fail"));
    EXPECT_THAT(output, testing::Not(testing::HasSubstr("recursive")));
}
