#include "infra/stream/StdVectorInputStream.hpp"
#include "gtest/gtest.h"
#include <cstdint>

TEST(StdVectorInputStreamTest, Extract)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ { 5, 6 } });

    uint8_t value;
    stream >> value;
    EXPECT_EQ(5, value);
}
