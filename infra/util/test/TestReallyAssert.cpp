#include "infra/util/ReallyAssert.hpp"
#include "gtest/gtest.h"

TEST(ReallyAssertTest, assert_passed_no_abort)
{
    really_assert(true); // Should not abort
}

TEST(ReallyAssertTest, assert_failed_abort)
{
    EXPECT_DEATH(really_assert(false), "");
}

TEST(ReallyAssertTest, assert_failed_handler)
{
    infra::RegisterAssertionFailureHandler([](auto, auto, auto)
        {
            std::cerr << "[assertion handler invoked]\n"
                      << std::flush;
        });

    // Manually calling handler because a debug build will call the standard assert instead of really_assert
    EXPECT_DEATH(infra::HandleAssertionFailure("condition", "file", 42), "\\[assertion handler invoked\\]");
}
