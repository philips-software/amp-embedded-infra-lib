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

    std::array<uint8_t, 2> data{ 1 << 3, 1 };
    builder.Feed(data);

    EXPECT_EQ(true, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_unknown_simple_field)
{
    services::ProtoMessageBuilder<test_messages::TestBool> builder;

    std::array<uint8_t, 4> data{ 2 << 3, 1, 1 << 3, 1 };
    builder.Feed(data);

    EXPECT_EQ(true, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_unknown_length_delimited)
{
    services::ProtoMessageBuilder<test_messages::TestBool> builder;

    std::array<uint8_t, 6> data{ 1 << 3, 1, (2 << 3) | 2, 2, 1 << 3, 0 };
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

TEST_F(ProtoMessageBuilderTest, parse_uint32)
{
    services::ProtoMessageBuilder<test_messages::TestUInt32> builder;

    std::array<uint8_t, 2> data{ 1 << 3, 5 };
    builder.Feed(data);

    EXPECT_EQ(5, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_int32)
{
    services::ProtoMessageBuilder<test_messages::TestInt32> builder;

    std::array<uint8_t, 11> data{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 };
    builder.Feed(data);

    EXPECT_EQ(-1, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_uint64)
{
    services::ProtoMessageBuilder<test_messages::TestUInt64> builder;

    std::array<uint8_t, 2> data{ 1 << 3, 5 };
    builder.Feed(data);

    EXPECT_EQ(5, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_int64)
{
    services::ProtoMessageBuilder<test_messages::TestInt64> builder;

    std::array<uint8_t, 11> data{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 };
    builder.Feed(data);

    EXPECT_EQ(-1, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_fixed32)
{
    services::ProtoMessageBuilder<test_messages::TestFixed32> builder;

    std::array<uint8_t, 5> data{ 13, 0x90, 0x91, 0x0f, 0 };
    builder.Feed(data);

    EXPECT_EQ(1020304, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_fixed64)
{
    services::ProtoMessageBuilder<test_messages::TestFixed64> builder;

    std::array<uint8_t, 9> data{ 9, 0x64, 0xc8, 0x0c, 0xce, 0xcb, 0x5c, 0x00, 0x00 };
    builder.Feed(data);

    EXPECT_EQ(102030405060708, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_sfixed32)
{
    services::ProtoMessageBuilder<test_messages::TestSFixed32> builder;

    std::array<uint8_t, 5> data{ 13, 0xff, 0xff, 0xff, 0xff };
    builder.Feed(data);

    EXPECT_EQ(-1, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_enum)
{
    services::ProtoMessageBuilder<test_messages::TestEnum> builder;

    std::array<uint8_t, 2> data{ 1 << 3, 1 };
    builder.Feed(data);

    EXPECT_EQ(test_messages::Enumeration::val1, builder.message.e);
}

TEST_F(ProtoMessageBuilderTest, parse_sfixed64)
{
    services::ProtoMessageBuilder<test_messages::TestSFixed64> builder;

    std::array<uint8_t, 9> data{ 9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    builder.Feed(data);

    EXPECT_EQ(-1, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_repeated_uint32)
{
    services::ProtoMessageBuilder<test_messages::TestRepeatedUInt32> builder;

    std::array<uint8_t, 4> data{ 1 << 3, 5, 1 << 3, 6 };
    builder.Feed(data);

    EXPECT_EQ((infra::BoundedVector<uint32_t>::WithMaxSize<10>{ { 5, 6 } }), builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_unbounded_repeated_uint32)
{
    services::ProtoMessageBuilder<test_messages::TestUnboundedRepeatedUInt32> builder;

    std::array<uint8_t, 4> data{ 1 << 3, 5, 1 << 3, 6 };
    builder.Feed(data);

    EXPECT_EQ((std::vector<uint32_t>{ { 5, 6 } }), builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_incomplete_int)
{
    services::ProtoMessageBuilder<test_messages::TestInt32> builder;

    std::array<uint8_t, 9> data{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    builder.Feed(data);
    EXPECT_EQ(0, builder.message.value);

    std::array<uint8_t, 2> data2{ 0xff, 1 };
    builder.Feed(data2);
    EXPECT_EQ(-1, builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_string)
{
    services::ProtoMessageBuilder<test_messages::TestString> builder;

    std::array<uint8_t, 6> data{ 10, 4, 'a', 'b', 'c', 'd' };
    builder.Feed(data);

    EXPECT_EQ("abcd", builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_std_string)
{
    services::ProtoMessageBuilder<test_messages::TestStdString> builder;

    std::array<uint8_t, 6> data{ 10, 4, 'a', 'b', 'c', 'd' };
    builder.Feed(data);

    EXPECT_EQ("abcd", builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_bytes)
{
    services::ProtoMessageBuilder<test_messages::TestBytes> builder;

    std::array<uint8_t, 4> data{ 10, 2, 5, 6 };
    builder.Feed(data);

    EXPECT_EQ((infra::BoundedVector<uint8_t>::WithMaxSize<10>{ { 5, 6 } }), builder.message.value);
}

TEST_F(ProtoMessageBuilderTest, parse_message)
{
    services::ProtoMessageBuilder<test_messages::TestMessageWithMessageField> builder;

    std::array<uint8_t, 4> data{ (1 << 3) | 2, 2, 1 << 3, 5 };
    builder.Feed(data);

    EXPECT_EQ((test_messages::TestMessageWithMessageField(test_messages::TestUInt32(5))), builder.message.message);
}

TEST_F(ProtoMessageBuilderTest, parse_more_nested_message)
{
    services::ProtoMessageBuilder<test_messages::TestMoreNestedMessage> builder;

    std::array<uint8_t, 8> data{ (1 << 3) | 2, 2, 1 << 3, 5, (2 << 3) | 2, 2, 2 << 3, 10 };
    builder.Feed(data);

    EXPECT_EQ((test_messages::TestMoreNestedMessage({ 5 }, { 10 })), builder.message);
}
