#include "generated/echo/TestMessages.pb.hpp"
#include "protobuf/echo/ProtoMessageBuilder.hpp"
#include <gtest/gtest.h>

class ProtoMessageBuilderTest
    : public testing::Test
{
public:
};

TEST_F(ProtoMessageBuilderTest, parse_bool)
{
    services::ProtoMessageBuilder<test_messages::TestBool> builder;

    std::array<uint8_t, 2> data{ 8, 1 };
    builder.Feed(data);

    EXPECT_EQ(true, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_incomplete_bool)
{
    services::ProtoMessageBuilder<test_messages::TestBool> builder;

    std::array<uint8_t, 1> data{ 8 };
    builder.Feed(data);

    EXPECT_EQ(false, builder.message.value);

    std::array<uint8_t, 1> data2{ 1 };
    builder.Feed(data2);

    EXPECT_EQ(true, builder.message.value);
}
