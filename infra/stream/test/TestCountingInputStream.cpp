#include "infra/stream/CountingInputStream.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "gmock/gmock.h"

TEST(CountingInputStreamTest, Extract)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::CountingStreamReader countingReader(reader);
    infra::StreamErrorPolicy errorPolicy;

    std::array<uint8_t, 4> data;
    EXPECT_CALL(reader, Extract(infra::MakeRange(data), testing::_));
    countingReader.Extract(data, errorPolicy);

    EXPECT_EQ(4, countingReader.TotalRead());
    EXPECT_FALSE(errorPolicy.Failed());
}

TEST(CountingInputStreamTest, Extract_but_fails)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::CountingStreamReader countingReader(reader);
    infra::StreamErrorPolicy errorPolicy(infra::noFail);

    std::array<uint8_t, 4> data;
    EXPECT_CALL(reader, Extract(infra::MakeRange(data), testing::_)).WillOnce(testing::Invoke([](infra::ByteRange range, infra::StreamErrorPolicy& errorPolicy) { errorPolicy.ReportResult(false); }));
    countingReader.Extract(data, errorPolicy);

    EXPECT_EQ(0, countingReader.TotalRead());
    EXPECT_TRUE(errorPolicy.Failed());
}

TEST(CountingInputStreamTest, Peek)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::CountingStreamReader countingReader(reader);
    infra::StreamErrorPolicy errorPolicy;

    EXPECT_CALL(reader, Peek(testing::Ref(errorPolicy))).WillOnce(testing::Return(4));
    EXPECT_EQ(4, countingReader.Peek(errorPolicy));

    EXPECT_EQ(0, countingReader.TotalRead());
}

TEST(CountingInputStreamTest, ExtractContiguousRange)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::CountingStreamReader countingReader(reader);

    std::array<uint8_t, 2> data;
    EXPECT_CALL(reader, ExtractContiguousRange(5)).WillOnce(testing::Return(infra::MakeRange(data)));
    EXPECT_EQ(infra::MakeRange(data), countingReader.ExtractContiguousRange(5));

    EXPECT_EQ(2, countingReader.TotalRead());
}

TEST(CountingInputStreamTest, PeekContiguousRange)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::CountingStreamReader countingReader(reader);

    std::array<uint8_t, 2> data{ { 1, 2 } };
    EXPECT_CALL(reader, PeekContiguousRange(7)).WillOnce(testing::Return(infra::MakeRange(data)));
    EXPECT_EQ(infra::MakeRange(data), countingReader.PeekContiguousRange(7));

    EXPECT_EQ(0, countingReader.TotalRead());
}

TEST(CountingInputStreamTest, Available)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::CountingStreamReader countingReader(reader);

    EXPECT_CALL(reader, Available()).WillOnce(testing::Return(4));
    EXPECT_EQ(4, countingReader.Available());

    EXPECT_EQ(0, countingReader.TotalRead());
}

TEST(CountingInputStreamTest, Empty)
{
    testing::StrictMock<infra::StreamReaderMock> reader;
    infra::CountingStreamReader countingReader(reader);

    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(false));
    EXPECT_FALSE(countingReader.Empty());

    EXPECT_CALL(reader, Empty()).WillOnce(testing::Return(true));
    EXPECT_TRUE(countingReader.Empty());

    EXPECT_EQ(0, countingReader.TotalRead());
}
