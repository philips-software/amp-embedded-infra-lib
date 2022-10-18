#include "infra/stream/BoundedDequeInputStream.hpp"
#include "gtest/gtest.h"

class BoundedDequeInputStreamReaderTest
    : public testing::Test
{
public:
    BoundedDequeInputStreamReaderTest()
        : reader(data)
    {
        data.pop_front();
        data.pop_front();
        data.push_back(7);
        data.push_back(8);
    }

    infra::BoundedDeque<uint8_t>::WithMaxSize<8> data{ std::initializer_list<uint8_t>{ 0, 0, 1, 2, 3, 4, 5, 6 } };
    infra::BoundedDequeInputStreamReader reader;
    infra::StreamErrorPolicy errorPolicy;
};

TEST_F(BoundedDequeInputStreamReaderTest, construction)
{
    EXPECT_FALSE(reader.Empty());
    EXPECT_EQ(8, reader.Available());
}

TEST_F(BoundedDequeInputStreamReaderTest, Extract)
{
    std::array<uint8_t, 4> result;
    reader.Extract(result, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), result);
    EXPECT_EQ(4, reader.Processed());

    reader.Extract(result, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 5, 6, 7, 8 } }), result);
    EXPECT_EQ(8, reader.Processed());
}

TEST_F(BoundedDequeInputStreamReaderTest, Peek)
{
    EXPECT_EQ(1, reader.Peek(errorPolicy));
}

TEST_F(BoundedDequeInputStreamReaderTest, ExtractContiguousRange)
{
    EXPECT_EQ((std::array<uint8_t, 6>{ { 1, 2, 3, 4, 5, 6 } }), reader.ExtractContiguousRange(16));
    EXPECT_EQ((std::array<uint8_t, 2>{ { 7, 8 } }), reader.ExtractContiguousRange(16));
}

TEST_F(BoundedDequeInputStreamReaderTest, PeekContiguousRange)
{
    EXPECT_EQ((std::array<uint8_t, 6>{ { 1, 2, 3, 4, 5, 6 } }), reader.PeekContiguousRange(0));
    EXPECT_EQ((std::array<uint8_t, 2>{ { 7, 8 } }), reader.PeekContiguousRange(6));

    reader.ExtractContiguousRange(2);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 3, 4, 5, 6 } }), reader.PeekContiguousRange(0));
}

TEST_F(BoundedDequeInputStreamReaderTest, ExtractContiguousRange_limited_by_max)
{
    EXPECT_EQ((std::array<uint8_t, 2>{ { 1, 2 } }), reader.ExtractContiguousRange(2));
}

TEST_F(BoundedDequeInputStreamReaderTest, Rewind)
{
    reader.ExtractContiguousRange(2);

    auto marker = reader.ConstructSaveMarker();
    reader.ExtractContiguousRange(2);
    reader.Rewind(marker);
    EXPECT_EQ((std::array<uint8_t, 2>{ { 3, 4 } }), reader.ExtractContiguousRange(2));
}

TEST(BoundedDequeInputStreamTest, construct_with_range)
{
    infra::BoundedDeque<uint8_t>::WithMaxSize<4> data{ std::initializer_list<uint8_t>{ 1, 2, 3, 4 } };
    infra::BoundedDequeInputStream stream(data);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), stream.ContiguousRange(16));
}

TEST(BoundedDequeInputStreamTest, construct_softFail_with_range)
{
    infra::BoundedDeque<uint8_t>::WithMaxSize<4> data{ std::initializer_list<uint8_t>{ 1, 2, 3, 4 } };
    infra::BoundedDequeInputStream stream(data, infra::softFail);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), stream.ContiguousRange(16));
}

TEST(BoundedDequeInputStreamTest, construct_noFail_with_range)
{
    infra::BoundedDeque<uint8_t>::WithMaxSize<4> data{ std::initializer_list<uint8_t>{ 1, 2, 3, 4 } };
    infra::BoundedDequeInputStream stream(data, infra::noFail);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), stream.ContiguousRange(16));
}
