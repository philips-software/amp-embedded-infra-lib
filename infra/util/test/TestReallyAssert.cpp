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
    really_assert(true); // Should not abort
}

TEST_F(ReallyAssertTest, assert_failed_abort)
{
    EXPECT_DEATH(really_assert(false), "");
}

TEST_F(ReallyAssertTest, assert_failed_without_handler)
{
    // Manually calling handler because a debug build will call the standard assert instead of really_assert
    infra::HandleAssertionFailure("condition", "file", 42); // Does nothing
}

TEST_F(ReallyAssertTest, assert_failed_with_handler)
{
    infra::RegisterAssertionFailureHandler([&](auto condition, auto file, auto line)
        {
            listener.OnAssertionFailure(condition, file, line);
        });
    EXPECT_CALL(listener, OnAssertionFailure("condition", "file", 42)).Times(1);

    // Manually calling handler because a debug build will call the standard assert instead of really_assert
    infra::HandleAssertionFailure("condition", "file", 42);
}
