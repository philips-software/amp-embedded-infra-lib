#include "generated/echo/TestMessages.pb.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/syntax/ProtoFormatter.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "gmock/gmock.h"

TEST(ProtoCEchoPluginTest, serialize_int32)
{
    test_messages::TestInt32 message;
    message.value = -1;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 11>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_int32)
{
    std::array<uint8_t, 11> data{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestInt32 message(parser);
    EXPECT_EQ(-1, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_int64)
{
    test_messages::TestInt64 message;
    message.value = -1;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 11>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_int64)
{
    std::array<uint8_t, 11> data{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestInt64 message(parser);
    EXPECT_EQ(-1, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_fixed32)
{
    test_messages::TestFixed32 message;
    message.value = 1020304;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 5>{ 13, 0x90, 0x91, 0x0f, 0 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_fixed32)
{
    std::array<uint8_t, 5> data{ 13, 0x90, 0x91, 0x0f, 0 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestFixed32 message(parser);
    EXPECT_EQ(1020304, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_fixed64)
{
    test_messages::TestFixed64 message;
    message.value = 102030405060708;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 9>{ 9, 0x64, 0xc8, 0x0c, 0xce, 0xcb, 0x5c, 0x00, 0x00 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_fixed64)
{
    std::array<uint8_t, 9> data{ 9, 0x64, 0xc8, 0x0c, 0xce, 0xcb, 0x5c, 0x00, 0x00 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestFixed64 message(parser);
    EXPECT_EQ(102030405060708, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_sfixed32)
{
    test_messages::TestSFixed32 message;
    message.value = -1;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 5>{ 13, 0xff, 0xff, 0xff, 0xff }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_sfixed32)
{
    std::array<uint8_t, 5> data{ 13, 0xff, 0xff, 0xff, 0xff };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestSFixed32 message(parser);
    EXPECT_EQ(-1, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_sfixed64)
{
    test_messages::TestSFixed64 message;
    message.value = -1;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 9>{ 9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_sfixed64)
{
    std::array<uint8_t, 9> data{ 9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestSFixed64 message(parser);
    EXPECT_EQ(-1, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_bool)
{
    test_messages::TestBool message;
    message.value = true;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 2>{ 8, 1 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_bool)
{
    std::array<uint8_t, 2> data{ 8, 1 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestBool message(parser);
    EXPECT_EQ(true, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_enum)
{
    test_messages::TestEnum message;
    message.e = test_messages::Enumeration::val1;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 2>{ 8, 1 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_enum)
{
    std::array<uint8_t, 2> data{ 8, 1 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestEnum message(parser);
    EXPECT_EQ(test_messages::Enumeration::val1, message.e);
}

TEST(ProtoCEchoPluginTest, serialize_string)
{
    test_messages::TestString message;
    message.value = "abcd";

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 6>{ 10, 4, 'a', 'b', 'c', 'd' }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_string)
{
    std::array<uint8_t, 6> data{ 10, 4, 'a', 'b', 'c', 'd' };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestString message(parser);
    EXPECT_EQ("abcd", message.value);
}

TEST(ProtoCEchoPluginTest, deserialize_corrupt_string)
{
    std::array<uint8_t, 6> data{ 10, 5, 'a', 'b', 'c', 'd' };
    infra::ByteInputStream stream(data, infra::softFail);
    infra::ProtoParser parser(stream);

    test_messages::TestString message(parser);
    EXPECT_TRUE(stream.Failed());
}

TEST(ProtoCEchoPluginTest, serialize_std_string)
{
    test_messages::TestStdString message;
    message.value = "abcd";

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 6>{ 10, 4, 'a', 'b', 'c', 'd' }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_std_string)
{
    std::array<uint8_t, 6> data{ 10, 4, 'a', 'b', 'c', 'd' };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestStdString message(parser);
    EXPECT_EQ("abcd", message.value);
}

TEST(ProtoCEchoPluginTest, serialize_repeated_string)
{
    test_messages::TestRepeatedString message;
    message.value.push_back("abcd");
    message.value.push_back("ef");

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 10>{ 10, 4, 'a', 'b', 'c', 'd', 10, 2, 'e', 'f' }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_repeated_string)
{
    std::array<uint8_t, 10> data{ 10, 4, 'a', 'b', 'c', 'd', 10, 2, 'e', 'f' };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestRepeatedString message(parser);
    infra::BoundedVector<infra::BoundedString::WithStorage<20>>::WithMaxSize<20> expected;
    expected.push_back("abcd");
    expected.push_back("ef");
    EXPECT_EQ(expected, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_bytes)
{
    test_messages::TestBytes message;
    message.value.push_back(5);
    message.value.push_back(6);

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 4>{ 10, 2, 5, 6 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_bytes)
{
    std::array<uint8_t, 4> data{ 10, 2, 5, 6 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestBytes message(parser);
    infra::BoundedVector<uint8_t>::WithMaxSize<10> value;
    value.push_back(5);
    value.push_back(6);
    EXPECT_EQ(value, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_unbounded_bytes)
{
    test_messages::TestUnboundedBytes message;
    message.value.push_back(5);
    message.value.push_back(6);

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 4>{ 10, 2, 5, 6 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_unbounded_bytes)
{
    std::array<uint8_t, 4> data{ 10, 2, 5, 6 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestUnboundedBytes message(parser);
    std::vector<uint8_t> value;
    value.push_back(5);
    value.push_back(6);
    EXPECT_EQ(value, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_uint32)
{
    test_messages::TestUInt32 message;
    message.value = 5;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 2>{ 1 << 3, 5 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_uint32)
{
    std::array<uint8_t, 2> data{ 1 << 3, 5 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestUInt32 message(parser);
    EXPECT_EQ(5, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_uint64)
{
    test_messages::TestUInt64 message;
    message.value = 5;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 2>{ 1 << 3, 5 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_uint64)
{
    std::array<uint8_t, 2> data{ 1 << 3, 5 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestUInt64 message(parser);
    EXPECT_EQ(5, message.value);
}

TEST(ProtoCEchoPluginTest, serialize_repeated_uint32)
{
    test_messages::TestRepeatedUInt32 message;
    message.value.push_back(5);
    message.value.push_back(6);

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 4>{ 1 << 3, 5, 1 << 3, 6 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_repeated_uint32)
{
    std::array<uint8_t, 4> data{ 1 << 3, 5, 1 << 3, 6 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestRepeatedUInt32 message(parser);
    EXPECT_EQ(2, message.value.size());
    EXPECT_EQ(5, message.value[0]);
    EXPECT_EQ(6, message.value[1]);
}

TEST(ProtoCEchoPluginTest, serialize_unbounded_repeated_uint32)
{
    test_messages::TestUnboundedRepeatedUInt32 message;
    message.value.push_back(5);
    message.value.push_back(6);

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 4>{ 1 << 3, 5, 1 << 3, 6 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_unbounded_repeated_uint32)
{
    std::array<uint8_t, 4> data{ 1 << 3, 5, 1 << 3, 6 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestUnboundedRepeatedUInt32 message(parser);
    EXPECT_EQ(2, message.value.size());
    EXPECT_EQ(5, message.value[0]);
    EXPECT_EQ(6, message.value[1]);
}

TEST(ProtoCEchoPluginTest, serialize_message)
{
    test_messages::TestMessageWithMessageField message;
    message.message.value = 5;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 4>{ (1 << 3) | 2, 2, 1 << 3, 5 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_message)
{
    std::array<uint8_t, 4> data{ (1 << 3) | 2, 2, 1 << 3, 5 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestMessageWithMessageField message(parser);
    EXPECT_EQ(5, message.message.value);
}

TEST(ProtoCEchoPluginTest, serialize_nested_message)
{
    test_messages::TestNestedMessage message;
    message.message.value = 5;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 4>{ (1 << 3) | 2, 2, 1 << 3, 5 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_nested_message)
{
    std::array<uint8_t, 4> data{ (1 << 3) | 2, 2, 1 << 3, 5 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestNestedMessage message(parser);
    EXPECT_EQ(5, message.message.value);
}

TEST(ProtoCEchoPluginTest, serialize_more_nested_message)
{
    test_messages::TestMoreNestedMessage message;
    message.message1.value = 5;
    message.message2.value = 10;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 8>{ (1 << 3) | 2, 2, 1 << 3, 5, (2 << 3) | 2, 2, 2 << 3, 10 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_more_nested_message)
{
    std::array<uint8_t, 8> data{ (1 << 3) | 2, 2, 1 << 3, 5, (2 << 3) | 2, 2, 2 << 3, 10 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestMoreNestedMessage message(parser);
    EXPECT_EQ(5, message.message1.value);
    EXPECT_EQ(10, message.message2.value);
}

TEST(ProtoCEchoPluginTest, serialize_nested_repeated_message)
{
    test_messages::TestNestedRepeatedMessage message;
    message.message.push_back(test_messages::TestNestedRepeatedMessage::NestedMessage());
    message.message[0].value = 5;
    message.message.push_back(test_messages::TestNestedRepeatedMessage::NestedMessage());
    message.message[1].value = 6;

    infra::ByteOutputStream::WithStorage<100> stream;
    infra::ProtoFormatter formatter(stream);
    message.Serialize(formatter);

    EXPECT_EQ((std::array<uint8_t, 8>{ (1 << 3) | 2, 2, 1 << 3, 5, (1 << 3) | 2, 2, 1 << 3, 6 }), stream.Writer().Processed());
}

TEST(ProtoCEchoPluginTest, deserialize_nested_repeated_message)
{
    std::array<uint8_t, 8> data{ (1 << 3) | 2, 2, 1 << 3, 5, (1 << 3) | 2, 2, 1 << 3, 6 };
    infra::ByteInputStream stream(data);
    infra::ProtoParser parser(stream);

    test_messages::TestNestedRepeatedMessage message(parser);
    EXPECT_EQ(5, message.message[0].value);
    EXPECT_EQ(6, message.message[1].value);
}
