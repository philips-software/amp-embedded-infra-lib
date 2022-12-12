#include "infra/stream/StdVectorInputStream.hpp"
#include "gtest/gtest.h"

class StdVectorInputStreamReaderTest
    : public testing::Test
{
public:
    std::vector<uint8_t> data{ 1, 2, 3, 4, 5, 6, 7, 8 };
    infra::StdVectorInputStreamReader reader{ data };
    infra::StreamErrorPolicy errorPolicy;
};

TEST_F(StdVectorInputStreamReaderTest, construction)
{
    EXPECT_FALSE(reader.Empty());
    EXPECT_EQ(8, reader.Available());
}

TEST_F(StdVectorInputStreamReaderTest, Extract)
{
    std::array<uint8_t, 4> result;
    reader.Extract(result, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 1, 2, 3, 4 } }), result);

    reader.Extract(result, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 4>{ { 5, 6, 7, 8 } }), result);
}

TEST_F(StdVectorInputStreamReaderTest, Peek)
{
    EXPECT_EQ(1, reader.Peek(errorPolicy));
}

TEST_F(StdVectorInputStreamReaderTest, ExtractContiguousRange)
{
    EXPECT_EQ((std::array<uint8_t, 6>{ { 1, 2, 3, 4, 5, 6 } }), reader.ExtractContiguousRange(6));
    EXPECT_EQ((std::array<uint8_t, 2>{ { 7, 8 } }), reader.ExtractContiguousRange(16));
}

TEST_F(StdVectorInputStreamReaderTest, PeekContiguousRange)
{
    EXPECT_EQ((std::array<uint8_t, 8>{ { 1, 2, 3, 4, 5, 6, 7, 8 } }), reader.PeekContiguousRange(0));

    reader.ExtractContiguousRange(2);
    EXPECT_EQ((std::array<uint8_t, 6>{ { 3, 4, 5, 6, 7, 8 } }), reader.PeekContiguousRange(0));
}

TEST_F(StdVectorInputStreamReaderTest, ExtractContiguousRange_limited_by_max)
{
    EXPECT_EQ((std::array<uint8_t, 2>{ { 1, 2 } }), reader.ExtractContiguousRange(2));
}

TEST_F(StdVectorInputStreamReaderTest, Rewind)
{
    reader.ExtractContiguousRange(2);

    auto marker = reader.ConstructSaveMarker();
    reader.ExtractContiguousRange(2);
    reader.Rewind(marker);
    EXPECT_EQ((std::array<uint8_t, 2>{ { 3, 4 } }), reader.ExtractContiguousRange(2));
}

TEST(StdVectorInputStreamTest, Extract)
{
    infra::StdVectorInputStream::WithStorage stream(infra::inPlace, std::vector<uint8_t>{ { 5, 6 } });

    uint8_t value;
    stream >> value;
    EXPECT_EQ(5, value);
}
