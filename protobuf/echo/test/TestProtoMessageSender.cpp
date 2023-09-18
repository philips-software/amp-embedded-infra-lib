#include "generated/echo/TestMessages.pb.hpp"
#include "infra/stream/ByteOutputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/stream/BoundedVectorOutputStream.hpp"
#include "protobuf/echo/ProtoMessageSender.hpp"
#include <gtest/gtest.h>

TEST(ProtoMessageSenderTest, format_bool)
{
    test_messages::TestBool message{ true };
    services::ProtoMessageSender sender(message);

    infra::StdVectorOutputStream::WithStorage stream;
    sender.Fill(stream);

    EXPECT_EQ((std::vector<uint8_t>{ 1 << 3, 1 }), stream.Storage());
}

TEST(ProtoMessageSenderTest, partially_format_bool)
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

TEST(ProtoMessageSenderTest, format_repeated_uint32)
{
    test_messages::TestRepeatedUInt32 message;
    message.value.push_back(5);
    message.value.push_back(6);
    services::ProtoMessageSender sender{ message };

    infra::StdVectorOutputStream::WithStorage stream;
    sender.Fill(stream);

    EXPECT_EQ((std::vector<uint8_t>{ 1 << 3, 5, 1 << 3, 6 }), stream.Storage());
}
