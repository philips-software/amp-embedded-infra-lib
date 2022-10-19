#include "upgrade/pack_builder/test_helper/ZeroFilledString.hpp"
#include "gtest/gtest.h"

TEST(ZeroFilledString, should_compare_true_to_matching_zero_filled_string)
{
    std::string compare{ 'f', 'o', 'o', 0, 0 };
    EXPECT_EQ(ZeroFilledString(5, "foo"), compare.c_str());
}

TEST(ZeroFilledString, should_compare_false_to_different_contents)
{
    std::string compare{ 'f', 'o', 'o', 0, 'f' };
    EXPECT_FALSE(ZeroFilledString(5, "foo") == compare.c_str());
}

TEST(ZeroFilledString, should_compare_false_to_different_size)
{
    std::string compare{ 'f', 'o', 'o' };
    EXPECT_FALSE(ZeroFilledString(5, "foo") == compare.c_str());
}
