#include "infra/stream/ByteInputStream.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <array>

class ByteInputStreamReaderTest
    : public testing::Test
{
public:
    ByteInputStreamReaderTest()
        : reader(data)
    {}

    std::array<uint8_t, 8> data{ { 1, 2, 3, 4, 5, 6, 7, 8 } };
    infra::ByteInputStreamReader reader;
    infra::StreamErrorPolicy errorPolicy;
};

TEST_F(ByteInputStreamReaderTest, construction)
{
    EXPECT_FALSE(reader.Empty());
    EXPECT_EQ(8, reader.Available());
}

TEST_F(ByteInputStreamReaderTest, Extract)
{
    std::array<uint8_t, 4> result;
    reader.Extract(result, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), result);
}

TEST_F(ByteInputStreamReaderTest, Peek)
{
    EXPECT_EQ(1, reader.Peek(errorPolicy));
}

TEST_F(ByteInputStreamReaderTest, Peek_at_end_reports_failure)
{
    infra::ByteInputStreamReader partialReader(infra::Head(infra::MakeRange(data), 4));
    infra::StreamErrorPolicy softFailErrorPolicy(infra::softFail);
    partialReader.ExtractContiguousRange(4);

    EXPECT_THAT(partialReader.Peek(softFailErrorPolicy), testing::Eq(0));
    EXPECT_TRUE(softFailErrorPolicy.Failed());
}

TEST_F(ByteInputStreamReaderTest, ExtractContiguousRange)
{
    EXPECT_EQ((std::array<uint8_t, 8>{ { 1, 2, 3, 4, 5, 6, 7, 8 } }), reader.ExtractContiguousRange(16));
}

TEST_F(ByteInputStreamReaderTest, PeekContiguousRange_from_start_position)
{
    EXPECT_EQ((std::array<uint8_t, 8>{ { 1, 2, 3, 4, 5, 6, 7, 8 } }), reader.PeekContiguousRange(0));

    EXPECT_EQ((std::array<uint8_t, 2>{ { 7, 8 } }), reader.PeekContiguousRange(6));
}

TEST_F(ByteInputStreamReaderTest, PeekContiguousRange_beyond_end_is_empty)
{
    reader.ExtractContiguousRange(6);

    EXPECT_THAT(reader.PeekContiguousRange(2), testing::IsEmpty());
    EXPECT_THAT(reader.PeekContiguousRange(3), testing::IsEmpty());
}

TEST_F(ByteInputStreamReaderTest, ExtractContiguousRange_limited_by_max)
{
    EXPECT_EQ((std::array<uint8_t, 2>{ { 1, 2 } }), reader.ExtractContiguousRange(2));
}

TEST_F(ByteInputStreamReaderTest, Rewind)
{
    reader.ExtractContiguousRange(2);

    auto marker = reader.ConstructSaveMarker();
    reader.ExtractContiguousRange(2);
    reader.Rewind(marker);
    EXPECT_EQ((std::array<uint8_t, 2>{ { 3, 4 } }), reader.ExtractContiguousRange(2));
}

TEST_F(ByteInputStreamReaderTest, ResetRange)
{
    std::array<uint8_t, 2> result;
    reader.Extract(result, errorPolicy);
    ASSERT_EQ((std::array<uint8_t, 2>{ { 1, 2 } }), result);

    std::array<uint8_t, 4> newData{ { 5, 6, 7, 8 } };
    reader.ResetRange(newData);

    reader.Extract(result, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 2>{ { 5, 6 } }), result);
}

TEST(ByteInputStreamTest, construct_with_range)
{
    std::array<uint8_t, 4> data{ { 1, 2, 3, 4 } };
    infra::ByteInputStream stream(data);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), stream.ContiguousRange(16));
}

TEST(ByteInputStreamTest, construct_softFail_with_range)
{
    std::array<uint8_t, 4> data{ { 1, 2, 3, 4 } };
    infra::ByteInputStream stream(data, infra::softFail);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), stream.ContiguousRange(16));
}

TEST(ByteInputStreamTest, construct_noFail_with_range)
{
    std::array<uint8_t, 4> data{ { 1, 2, 3, 4 } };
    infra::ByteInputStream stream(data, infra::noFail);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), stream.ContiguousRange(16));
}

TEST(ByteInputStreamTest, StreamFromRange)
{
    std::array<uint8_t, 4> from = { 0, 1, 2, 3 };

    struct To
    {
        uint8_t a;
        uint8_t b;

        bool operator==(const To& other) const
        {
            return a == other.a && b == other.b;
        };
    } to = { 4, 5 };

    infra::ByteInputStream stream(from);

    EXPECT_EQ(from, stream.Reader().Remaining());

    stream >> to;
    EXPECT_EQ((To{ 0, 1 }), to);
    EXPECT_FALSE(stream.Empty());

    stream >> to;
    EXPECT_EQ((To{ 2, 3 }), to);
    EXPECT_TRUE(stream.Empty());
    EXPECT_EQ(from, stream.Reader().Processed());
}

TEST(ByteInputStreamTest, StreamToMemoryRange)
{
    std::array<uint8_t, 2> to = { 0, 1 };
    std::array<uint8_t, 4> buffer = { 2, 3, 4, 5 };

    infra::ByteInputStream stream(buffer);
    stream >> to;

    EXPECT_EQ((std::array<uint8_t, 2>{ { 2, 3 } }), to);
}
