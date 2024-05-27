#include "generated/echo/TestMessages.pb.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "protobuf/echo/ProtoMessageReceiver.hpp"
#include <gtest/gtest.h>

TEST(ProtoMessageReceiverTest, parse_bool)
{
    services::ProtoMessageReceiver<test_messages::TestBool> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 1 << 3, 1 });
    receiver.Feed(data);

    EXPECT_EQ(true, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_unknown_simple_field)
{
    services::ProtoMessageReceiver<test_messages::TestBool> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 2 << 3, 1, 1 << 3, 1 });
    receiver.Feed(data);

    EXPECT_EQ(true, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_unknown_length_delimited)
{
    services::ProtoMessageReceiver<test_messages::TestBool> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 1 << 3, 1, (2 << 3) | 2, 2, 1 << 3, 0 });
    receiver.Feed(data);

    EXPECT_EQ(true, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_incomplete_bool)
{
    services::ProtoMessageReceiver<test_messages::TestBool> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 8 });
    receiver.Feed(data);

    EXPECT_EQ(false, receiver.message.value);

    infra::StdVectorInputStreamReader::WithStorage data2(std::in_place, std::initializer_list<uint8_t>{ 1 });
    receiver.Feed(data2);

    EXPECT_EQ(true, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_uint32)
{
    services::ProtoMessageReceiver<test_messages::TestUInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 1 << 3, 5 });
    receiver.Feed(data);

    EXPECT_EQ(5, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_int32)
{
    services::ProtoMessageReceiver<test_messages::TestInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 });
    receiver.Feed(data);

    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_uint64)
{
    services::ProtoMessageReceiver<test_messages::TestUInt64> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 1 << 3, 5 });
    receiver.Feed(data);

    EXPECT_EQ(5, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_int64)
{
    services::ProtoMessageReceiver<test_messages::TestInt64> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 });
    receiver.Feed(data);

    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_fixed32)
{
    services::ProtoMessageReceiver<test_messages::TestFixed32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 13, 0x90, 0x91, 0x0f, 0 });
    receiver.Feed(data);

    EXPECT_EQ(1020304, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_fixed64)
{
    services::ProtoMessageReceiver<test_messages::TestFixed64> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 9, 0x64, 0xc8, 0x0c, 0xce, 0xcb, 0x5c, 0x00, 0x00 });
    receiver.Feed(data);

    EXPECT_EQ(102030405060708, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_sfixed32)
{
    services::ProtoMessageReceiver<test_messages::TestSFixed32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 13, 0xff, 0xff, 0xff, 0xff });
    receiver.Feed(data);

    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_sfixed64)
{
    services::ProtoMessageReceiver<test_messages::TestSFixed64> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff });
    receiver.Feed(data);

    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_enum)
{
    services::ProtoMessageReceiver<test_messages::TestEnum> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 1 << 3, 1 });
    receiver.Feed(data);

    EXPECT_EQ(test_messages::Enumeration::val1, receiver.message.e);
}

TEST(ProtoMessageReceiverTest, parse_repeated_uint32)
{
    services::ProtoMessageReceiver<test_messages::TestRepeatedUInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 1 << 3, 5, 1 << 3, 6 });
    receiver.Feed(data);

    EXPECT_EQ((infra::BoundedVector<uint32_t>::WithMaxSize<10>{ { 5, 6 } }), receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_unbounded_repeated_uint32)
{
    services::ProtoMessageReceiver<test_messages::TestUnboundedRepeatedUInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 1 << 3, 5, 1 << 3, 6 });
    receiver.Feed(data);

    EXPECT_EQ((std::vector<uint32_t>{ { 5, 6 } }), receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_incomplete_int)
{
    services::ProtoMessageReceiver<test_messages::TestInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff });
    receiver.Feed(data);
    EXPECT_EQ(0, receiver.message.value);

    infra::StdVectorInputStreamReader::WithStorage data2(std::in_place, std::initializer_list<uint8_t>{ 0xff, 1 });
    receiver.Feed(data2);
    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_string)
{
    services::ProtoMessageReceiver<test_messages::TestString> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 10, 4, 'a', 'b', 'c', 'd' });
    receiver.Feed(data);

    EXPECT_EQ("abcd", receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_std_string)
{
    services::ProtoMessageReceiver<test_messages::TestStdString> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 10, 4, 'a', 'b', 'c', 'd' });
    receiver.Feed(data);

    EXPECT_EQ("abcd", receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_bytes)
{
    services::ProtoMessageReceiver<test_messages::TestBytes> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ 10, 2, 5, 6 });
    receiver.Feed(data);

    EXPECT_EQ((infra::BoundedVector<uint8_t>::WithMaxSize<10>{ { 5, 6 } }), receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_message)
{
    services::ProtoMessageReceiver<test_messages::TestMessageWithMessageField> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ (1 << 3) | 2, 2, 1 << 3, 5 });
    receiver.Feed(data);

    EXPECT_EQ((test_messages::TestMessageWithMessageField(test_messages::TestUInt32(5))), receiver.message.message);
}

TEST(ProtoMessageReceiverTest, parse_more_nested_message)
{
    services::ProtoMessageReceiver<test_messages::TestMoreNestedMessage> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(std::in_place, std::initializer_list<uint8_t>{ (1 << 3) | 2, 2, 1 << 3, 5, (2 << 3) | 2, 2, 2 << 3, 10 });
    receiver.Feed(data);

    EXPECT_EQ((test_messages::TestMoreNestedMessage({ 5 }, { 10 })), receiver.message);
}
