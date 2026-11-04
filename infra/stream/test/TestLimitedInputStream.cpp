#include "infra/stream/BoundedDequeInputStream.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "gmock/gmock.h"

TEST(LimitedInputStreamTest, Extract)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::LimitedStreamReader limitedReader(reader, 2);
    infra::StreamErrorPolicy errorPolicy(infra::softFail);

    std::array<uint8_t, 4> data;
    EXPECT_CALL(reader, Extract(infra::Head(infra::MakeRange(data), 2), testing::Ref(errorPolicy)));
    limitedReader.Extract(data, errorPolicy);
    EXPECT_TRUE(errorPolicy.Failed());
}

TEST(LimitedInputStreamTest, Peek_with_data_available)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::LimitedStreamReader limitedReader(reader, 1);
    infra::StreamErrorPolicy errorPolicy(infra::softFail);

    EXPECT_CALL(reader, Peek(testing::Ref(errorPolicy))).WillOnce(testing::Return(4));
    EXPECT_EQ(4, limitedReader.Peek(errorPolicy));
    EXPECT_FALSE(errorPolicy.Failed());
}

TEST(LimitedInputStreamTest, Peek_with_no_data_available)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::LimitedStreamReader limitedReader(reader, 0);
    infra::StreamErrorPolicy errorPolicy(infra::softFail);

    EXPECT_EQ(0, limitedReader.Peek(errorPolicy));
    EXPECT_TRUE(errorPolicy.Failed());
}

TEST(LimitedInputStreamTest, ExtractContiguousRange)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::LimitedStreamReader limitedReader(reader, 2);

    std::array<uint8_t, 2> data;
    EXPECT_CALL(reader, ExtractContiguousRange(2)).WillOnce(testing::Return(infra::MakeRange(data)));
    EXPECT_EQ(infra::MakeRange(data), limitedReader.ExtractContiguousRange(4));

    EXPECT_TRUE(limitedReader.Empty());
}

TEST(LimitedInputStreamTest, PeekContiguousRange)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::LimitedStreamReader limitedReader(reader, 2);

    std::array<uint8_t, 2> data{ { 1, 2 } };
    EXPECT_CALL(reader, PeekContiguousRange(0)).WillOnce(testing::Return(infra::MakeRange(data)));
    EXPECT_EQ(infra::MakeRange(data), limitedReader.PeekContiguousRange(0));

    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(false));
    EXPECT_FALSE(limitedReader.Empty());
}

TEST(LimitedInputStreamTest, Available)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::LimitedStreamReader limitedReader(reader, 2);

    EXPECT_CALL(reader, Available()).WillOnce(testing::Return(4));
    EXPECT_EQ(2, limitedReader.Available());

    EXPECT_CALL(reader, Available()).WillOnce(testing::Return(1));
    EXPECT_EQ(1, limitedReader.Available());
}

TEST(LimitedInputStreamTest, Empty)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::LimitedStreamReader limitedReader(reader, 2);

    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(false));
    EXPECT_FALSE(limitedReader.Empty());

    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(true));
    EXPECT_TRUE(limitedReader.Empty());
}

TEST(LimitedInputStreamTest, ResetLength)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::LimitedStreamReader limitedReader(reader, 2);

    limitedReader.ResetLength(3);

    EXPECT_CALL(reader, Available()).WillOnce(testing::Return(4));
    EXPECT_EQ(3, limitedReader.Available());
}

TEST(LimitedInputStreamWithRewindingTest, Rewind_after_partial_Extract_restores_length)
{
    infra::BoundedDeque<uint8_t>::WithMaxSize<8> data{ std::initializer_list<uint8_t>{ 1, 2, 3, 4 } };
    infra::BoundedDequeInputStreamReader reader(data);
    infra::LimitedStreamReaderWithRewinding limitedReader(reader, 6);
    infra::StreamErrorPolicy errorPolicy(infra::softFail);

    auto marker = limitedReader.ConstructSaveMarker();
    std::array<uint8_t, 6> buffer{};
    limitedReader.Extract(buffer, errorPolicy);
    limitedReader.Rewind(marker);

    data.push_back(5);
    data.push_back(6);

    EXPECT_TRUE(errorPolicy.Failed());
    EXPECT_THAT(limitedReader.Available(), testing::Eq(6));
}

template<class LimitedReader>
class LimitedStreamReaderPeekTest
    : public testing::Test
{
public:
    std::array<uint8_t, 6> data{ { 1, 2, 3, 4, 5, 6 } };
    infra::ByteInputStreamReader reader{ data };
    LimitedReader limitedReader{ reader, 2 };
};

using LimitedReaderTypes = testing::Types<infra::LimitedStreamReader, infra::LimitedStreamReaderWithRewinding>;
TYPED_TEST_SUITE(LimitedStreamReaderPeekTest, LimitedReaderTypes);

TYPED_TEST(LimitedStreamReaderPeekTest, PeekContiguousRange_does_not_exceed_length)
{
    EXPECT_THAT(this->limitedReader.PeekContiguousRange(0), testing::ElementsAre(1, 2));
    EXPECT_THAT(this->limitedReader.PeekContiguousRange(1), testing::ElementsAre(2));
    EXPECT_THAT(this->limitedReader.PeekContiguousRange(2), testing::IsEmpty());
}

TYPED_TEST(LimitedStreamReaderPeekTest, PeekContiguousRange_does_not_exceed_remaining_length_after_extract)
{
    this->limitedReader.ExtractContiguousRange(1);

    EXPECT_THAT(this->limitedReader.PeekContiguousRange(0), testing::ElementsAre(2));
    EXPECT_THAT(this->limitedReader.PeekContiguousRange(1), testing::IsEmpty());
}
