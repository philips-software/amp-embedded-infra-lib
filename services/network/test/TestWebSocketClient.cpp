#include "gmock/gmock.h"
#include "infra/stream/ByteInputStream.hpp"
#include "services/network/WebSocket.hpp"

class WebSocketClientTest
    : public testing::Test
{};

TEST_F(WebSocketClientTest, should_parse_valid_websocket_header)
{
    std::vector<uint8_t> data({ 0x80, 0x80, 0x00, 0x00, 0x00, 0x00 });
    infra::ByteInputStream stream(data);
    services::WebSocketFrameHeader header(stream.Reader());

    EXPECT_TRUE(header.IsValid());
    EXPECT_TRUE(header.IsComplete());
    EXPECT_TRUE(header.IsFinalFrame());
}

TEST_F(WebSocketClientTest, should_parse_masking_key)
{
    std::vector<uint8_t> data({ 0x80, 0x80, 0xAB, 0xCD, 0xEF, 0x01 });
    infra::ByteInputStream stream(data);
    services::WebSocketFrameHeader header(stream.Reader());

    std::array<uint8_t, 4> expected = { 0xAB, 0xCD, 0xEF, 0x01 };
    EXPECT_EQ(expected, header.MaskingKey());
}

TEST_F(WebSocketClientTest, should_parse_payload_length)
{
    std::vector<uint8_t> data({ 0x80, 0xFD, 0x00, 0x00, 0x00, 0x00 });
    infra::ByteInputStream stream(data);
    services::WebSocketFrameHeader header(stream.Reader());

    EXPECT_EQ(125, header.PayloadLength());
}

TEST_F(WebSocketClientTest, should_parse_extended16_payload_length)
{
    std::vector<uint8_t> data({ 0x80, 0xFE, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 });
    infra::ByteInputStream stream(data);
    services::WebSocketFrameHeader header(stream.Reader());

    EXPECT_EQ(std::numeric_limits<uint16_t>::max(), header.PayloadLength());
}

TEST_F(WebSocketClientTest, should_parse_extended64_payload_length)
{
    std::vector<uint8_t> data({ 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 });
    infra::ByteInputStream stream(data);
    services::WebSocketFrameHeader header(stream.Reader());

    EXPECT_EQ(std::numeric_limits<uint64_t>::max(), header.PayloadLength());
}
