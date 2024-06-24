#include "generated/echo/TestMessages.pb.hpp"
#include "infra/stream/StdVectorInputStream.hpp"
#include "protobuf/echo/ProtoMessageReceiver.hpp"
#include <gtest/gtest.h>

TEST(ProtoMessageReceiverTest, parse_bool)
{
    services::ProtoMessageReceiver<test_messages::TestBool> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 1 << 3, 1 });
    receiver.Feed(data);

    EXPECT_EQ(true, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_unknown_simple_field)
{
    services::ProtoMessageReceiver<test_messages::TestBool> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 2 << 3, 1, 1 << 3, 1 });
    receiver.Feed(data);

    EXPECT_EQ(true, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_unknown_length_delimited)
{
    services::ProtoMessageReceiver<test_messages::TestBool> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 1 << 3, 1, (2 << 3) | 2, 2, 1 << 3, 0 });
    receiver.Feed(data);

    EXPECT_EQ(true, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_incomplete_bool)
{
    services::ProtoMessageReceiver<test_messages::TestBool> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 8 });
    receiver.Feed(data);

    EXPECT_EQ(false, receiver.message.value);

    infra::StdVectorInputStreamReader::WithStorage data2(infra::inPlace, std::initializer_list<uint8_t>{ 1 });
    receiver.Feed(data2);

    EXPECT_EQ(true, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_uint32)
{
    services::ProtoMessageReceiver<test_messages::TestUInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 1 << 3, 5 });
    receiver.Feed(data);

    EXPECT_EQ(5, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_int32)
{
    services::ProtoMessageReceiver<test_messages::TestInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 });
    receiver.Feed(data);

    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_uint64)
{
    services::ProtoMessageReceiver<test_messages::TestUInt64> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 1 << 3, 5 });
    receiver.Feed(data);

    EXPECT_EQ(5, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_int64)
{
    services::ProtoMessageReceiver<test_messages::TestInt64> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 });
    receiver.Feed(data);

    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_fixed32)
{
    services::ProtoMessageReceiver<test_messages::TestFixed32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 13, 0x90, 0x91, 0x0f, 0 });
    receiver.Feed(data);

    EXPECT_EQ(1020304, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_fixed64)
{
    services::ProtoMessageReceiver<test_messages::TestFixed64> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 9, 0x64, 0xc8, 0x0c, 0xce, 0xcb, 0x5c, 0x00, 0x00 });
    receiver.Feed(data);

    EXPECT_EQ(102030405060708, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_sfixed32)
{
    services::ProtoMessageReceiver<test_messages::TestSFixed32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 13, 0xff, 0xff, 0xff, 0xff });
    receiver.Feed(data);

    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_sfixed64)
{
    services::ProtoMessageReceiver<test_messages::TestSFixed64> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff });
    receiver.Feed(data);

    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_enum)
{
    services::ProtoMessageReceiver<test_messages::TestEnum> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 1 << 3, 1 });
    receiver.Feed(data);

    EXPECT_EQ(test_messages::Enumeration::val1, receiver.message.e);
}

TEST(ProtoMessageReceiverTest, parse_repeated_uint32)
{
    services::ProtoMessageReceiver<test_messages::TestRepeatedUInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 1 << 3, 5, 1 << 3, 6 });
    receiver.Feed(data);

    EXPECT_EQ((infra::BoundedVector<uint32_t>::WithMaxSize<10>{ { 5, 6 } }), receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_unbounded_repeated_uint32)
{
    services::ProtoMessageReceiver<test_messages::TestUnboundedRepeatedUInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 1 << 3, 5, 1 << 3, 6 });
    receiver.Feed(data);

    EXPECT_EQ((std::vector<uint32_t>{ { 5, 6 } }), receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_incomplete_int)
{
    services::ProtoMessageReceiver<test_messages::TestInt32> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff });
    receiver.Feed(data);
    EXPECT_EQ(0, receiver.message.value);

    infra::StdVectorInputStreamReader::WithStorage data2(infra::inPlace, std::initializer_list<uint8_t>{ 0xff, 1 });
    receiver.Feed(data2);
    EXPECT_EQ(-1, receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_string)
{
    services::ProtoMessageReceiver<test_messages::TestString> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 10, 4, 'a', 'b', 'c', 'd' });
    receiver.Feed(data);

    EXPECT_EQ("abcd", receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_std_string)
{
    services::ProtoMessageReceiver<test_messages::TestStdString> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 10, 4, 'a', 'b', 'c', 'd' });
    receiver.Feed(data);

    EXPECT_EQ("abcd", receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_bytes)
{
    services::ProtoMessageReceiver<test_messages::TestBytes> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ 10, 2, 5, 6 });
    receiver.Feed(data);

    EXPECT_EQ((infra::BoundedVector<uint8_t>::WithMaxSize<10>{ { 5, 6 } }), receiver.message.value);
}

TEST(ProtoMessageReceiverTest, parse_message)
{
    services::ProtoMessageReceiver<test_messages::TestMessageWithMessageField> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ (1 << 3) | 2, 2, 1 << 3, 5 });
    receiver.Feed(data);

    EXPECT_EQ((test_messages::TestMessageWithMessageField(test_messages::TestUInt32(5))), receiver.message.message);
}

TEST(ProtoMessageReceiverTest, parse_more_nested_message)
{
    services::ProtoMessageReceiver<test_messages::TestMoreNestedMessage> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{ (1 << 3) | 2, 2, 1 << 3, 5, (2 << 3) | 2, 2, 2 << 3, 10 });
    receiver.Feed(data);

    EXPECT_EQ((test_messages::TestMoreNestedMessage({ 5 }, { 10 })), receiver.message);
}

TEST(ProtoMessageReceiverTest, parse_optionals_all_none)
{
    services::ProtoMessageReceiver<test_messages::TestOptionalEverything> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{});
    receiver.Feed(data);

    EXPECT_EQ(infra::none, receiver.message.v0);
    EXPECT_EQ(infra::none, receiver.message.v1);
    EXPECT_EQ(infra::none, receiver.message.v2);
    EXPECT_EQ(infra::none, receiver.message.v3);
    EXPECT_EQ(infra::none, receiver.message.v4);
    EXPECT_EQ(infra::none, receiver.message.v5);
    EXPECT_EQ(infra::none, receiver.message.v6);
    EXPECT_EQ(infra::none, receiver.message.v7);
    EXPECT_EQ(infra::none, receiver.message.v8);
    EXPECT_EQ(infra::none, receiver.message.v9);
    EXPECT_EQ(infra::none, receiver.message.v10);
    EXPECT_EQ(infra::none, receiver.message.v11);
    EXPECT_EQ(infra::none, receiver.message.v12);
    EXPECT_EQ(infra::none, receiver.message.v13);
    EXPECT_EQ(infra::none, receiver.message.v14);
}

TEST(ProtoMessageReceiverTest, parse_optionals_all_filled)
{
    services::ProtoMessageReceiver<test_messages::TestOptionalEverything> receiver;

    infra::StdVectorInputStreamReader::WithStorage data(infra::inPlace, std::initializer_list<uint8_t>{
                                                                            0x08, 0x01, 0x10, 0x02, 0x18, 0x03, 0x20, 0x04,
                                                                            0x28, 0x05, 0x35, 0x06, 0x00, 0x00, 0x00, 0x39,
                                                                            0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                            0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                            0x00, 0x4d, 0x09, 0x00, 0x00, 0x00, 0x50, 0x01,
                                                                            0x5a, 0x01, 0x61, 0x62, 0x01, 0x62, 0x6a, 0x02,
                                                                            0x08, 0x0a, 0x72, 0x02, 0x0b, 0x0b, 0x7a, 0x02,
                                                                            0x0c, 0x0c });
    receiver.Feed(data);

    EXPECT_EQ(test_messages::Enumeration::val1, *receiver.message.v0);
    EXPECT_EQ(2, *receiver.message.v1);
    EXPECT_EQ(3, *receiver.message.v2);
    EXPECT_EQ(4, *receiver.message.v3);
    EXPECT_EQ(5, *receiver.message.v4);
    EXPECT_EQ(6, *receiver.message.v5);
    EXPECT_EQ(7, *receiver.message.v6);
    EXPECT_EQ(8, *receiver.message.v7);
    EXPECT_EQ(9, *receiver.message.v8);
    EXPECT_EQ(true, *receiver.message.v9);
    EXPECT_EQ("a", *receiver.message.v10);
    EXPECT_EQ("b", *receiver.message.v11);
    EXPECT_EQ(test_messages::TestUInt32(10), *receiver.message.v12);
    EXPECT_EQ((infra::BoundedVector<uint8_t>::WithMaxSize<2>(static_cast<std::size_t>(2), 11)), *receiver.message.v13);
    EXPECT_EQ((std::vector<uint8_t>(2, 12)), *receiver.message.v14);
}
