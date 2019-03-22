#include "gtest/gtest.h"
#include "infra/stream/StdStringOutputStream.hpp"
#include <cstdint>

TEST(StdStringOuputStreamTest, StdStringOutputStream)
{
    infra::StdStringOutputStream::WithStorage stream;

    stream << "abcd";

    EXPECT_EQ("abcd", stream.Storage());
}

TEST(StdStringOuputStreamTest, available_returns_max)
{
    infra::StdStringOutputStream::WithStorage stream;
    EXPECT_EQ(std::numeric_limits<size_t>::max(), stream.Available());
}
