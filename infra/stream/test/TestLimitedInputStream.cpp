#include "gmock/gmock.h"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/stream/test/StreamMock.hpp"

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
