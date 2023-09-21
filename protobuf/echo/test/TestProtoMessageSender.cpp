#include "generated/echo/TestMessages.pb.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "infra/util/ConstructBin.hpp"
#include "protobuf/echo/ProtoMessageSender.hpp"
#include <gtest/gtest.h>

class ProtoMessageSenderTest
    : public testing::Test
{
public:
    template<class Message>
    void ExpectFill(const std::vector<uint8_t>& data, services::ProtoMessageSender<Message>& sender)
    {
        infra::StdVectorOutputStream::WithStorage stream;
        sender.Fill(stream);

        EXPECT_EQ(data, stream.Storage());
    }
};

TEST_F(ProtoMessageSenderTest, format_bool)
{
    test_messages::TestBool message{ true };
    services::ProtoMessageSender sender(message);

    ExpectFill({ 1 << 3, 1 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_uint32)
{
    test_messages::TestUInt32 message(5);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 1 << 3, 5 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_int32)
{
    test_messages::TestInt32 message(-1);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_uint64)
{
    test_messages::TestUInt64 message(5);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 1 << 3, 5 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_int64)
{
    test_messages::TestInt64 message(-1);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_fixed32)
{
    test_messages::TestFixed32 message(1020304);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 13, 0x90, 0x91, 0x0f, 0 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_fixed64)
{
    test_messages::TestFixed64 message(102030405060708);;
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 9, 0x64, 0xc8, 0x0c, 0xce, 0xcb, 0x5c, 0x00, 0x00 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_sfixed32)
{
    test_messages::TestSFixed32 message(-1);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 13, 0xff, 0xff, 0xff, 0xff }, sender);
}

TEST_F(ProtoMessageSenderTest, format_sfixed64)
{
    test_messages::TestSFixed64 message(-1);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, sender);
}

TEST_F(ProtoMessageSenderTest, partially_format_bool)
{
    test_messages::TestBool message{ true };
    services::ProtoMessageSender sender(message);

    infra::ByteOutputStream::WithStorage<1> partialStream;
    sender.Fill(partialStream);
    infra::StdVectorOutputStream::WithStorage stream;
    sender.Fill(stream);

    EXPECT_EQ((std::array<uint8_t, 1>{ 1 << 3 }), partialStream.Storage());
    EXPECT_EQ((std::vector<uint8_t>{ 1 }), stream.Storage());
}

TEST_F(ProtoMessageSenderTest, format_enum)
{
    test_messages::TestEnum message{ test_messages::Enumeration::val1 };
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 1 << 3, 1 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_repeated_uint32)
{
    test_messages::TestRepeatedUInt32 message;
    message.value.push_back(5);
    message.value.push_back(6);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 1 << 3, 5, 1 << 3, 6 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_unbounded_repeated_uint32)
{
    test_messages::TestUnboundedRepeatedUInt32 message;
    message.value.push_back(5);
    message.value.push_back(6);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 1 << 3, 5, 1 << 3, 6 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_string)
{
    test_messages::TestString message{ "abcd" };
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 10, 4, 'a', 'b', 'c', 'd' }, sender);
}

TEST_F(ProtoMessageSenderTest, format_std_string)
{
    test_messages::TestStdString message{ "abcd" };
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 10, 4, 'a', 'b', 'c', 'd' }, sender);
}

TEST_F(ProtoMessageSenderTest, format_bytes)
{
    test_messages::TestBytes message;
    message.value.push_back(5);
    message.value.push_back(6);
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 10, 2, 5, 6 }, sender);

}

TEST_F(ProtoMessageSenderTest, format_message)
{
    test_messages::TestMessageWithMessageField message{ 5 };
    services::ProtoMessageSender sender{ message };

    ExpectFill({ 1 << 3 | 2, 2, 1 << 3, 5 }, sender);
}

TEST_F(ProtoMessageSenderTest, format_more_nested_message)
{
    test_messages::TestMoreNestedMessage message{ { 5 }, { 10 } };
    services::ProtoMessageSender sender{ message };

    ExpectFill({ (1 << 3) | 2, 2, 1 << 3, 5, (2 << 3) | 2, 2, 2 << 3, 10 }, sender);
}

TEST_F(ProtoMessageSenderTest, dont_format_on_buffer_full)
{
    test_messages::TestBoolWithBytes message;
    message.b.insert(message.b.begin(), 30, static_cast<uint8_t>(5));
    message.value = true;
    services::ProtoMessageSender sender{ message };

    infra::ByteOutputStream::WithStorage<1> partialStream;
    sender.Fill(partialStream);

    infra::StdVectorOutputStream::WithStorage stream;
    sender.Fill(stream);
    EXPECT_EQ((std::vector<uint8_t>{ 30, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2 << 3, 1 }), stream.Storage());
}

TEST_F(ProtoMessageSenderTest, format_many_repeated_uint32)
{
    test_messages::TestRepeatedUInt32 message;
    message.value.insert(message.value.end(), 50, static_cast<uint8_t>(5));
    services::ProtoMessageSender sender{ message };

    infra::ByteOutputStream::WithStorage<10> partialStream;
    sender.Fill(partialStream);
    EXPECT_EQ((std::array<uint8_t, 10>{ 1 << 3, 5, 1 << 3, 5, 1 << 3, 5, 1 << 3, 5, 1 << 3, 5 }), partialStream.Storage());

    infra::StdVectorOutputStream::WithStorage stream;
    sender.Fill(stream);
    EXPECT_EQ(infra::ConstructBin().Repeat(45, { 1 << 3, 5 }).Vector(), stream.Storage());
}

TEST_F(ProtoMessageSenderTest, format_many_bytes)
{
    test_messages::TestBytes message;
    message.value.insert(message.value.end(), 50, static_cast<uint8_t>(5));
    services::ProtoMessageSender sender{ message };

    infra::ByteOutputStream::WithStorage<10> partialStream;
    sender.Fill(partialStream);
    EXPECT_EQ((std::array<uint8_t, 10>{ 10, 50, 5, 5, 5, 5, 5, 5, 5, 5 }), partialStream.Storage());

    infra::StdVectorOutputStream::WithStorage stream;
    sender.Fill(stream);
    EXPECT_EQ((std::vector<uint8_t>{ 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 }), stream.Storage());
}

