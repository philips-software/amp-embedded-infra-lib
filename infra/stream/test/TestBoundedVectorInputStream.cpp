#include "infra/stream/BoundedVectorInputStream.hpp"
#include "gmock/gmock.h"

class BoundedVectorInputStreamReaderTest
    : public testing::Test
{
public:
    infra::BoundedVector<uint8_t>::WithMaxSize<8> data{ std::initializer_list<uint8_t>{ 1, 2, 3, 4 } };
    infra::BoundedVectorInputStreamReader reader{ data };
};

TEST_F(BoundedVectorInputStreamReaderTest, PeekContiguousRange)
{
    EXPECT_THAT(reader.PeekContiguousRange(0), testing::ElementsAre(1, 2, 3, 4));
    EXPECT_THAT(reader.PeekContiguousRange(3), testing::ElementsAre(4));
}

TEST_F(BoundedVectorInputStreamReaderTest, PeekContiguousRange_beyond_end_is_empty)
{
    reader.ExtractContiguousRange(2);

    EXPECT_THAT(reader.PeekContiguousRange(2), testing::IsEmpty());
    EXPECT_THAT(reader.PeekContiguousRange(3), testing::IsEmpty());
}
