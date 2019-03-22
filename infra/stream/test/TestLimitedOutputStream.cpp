#include "gmock/gmock.h"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/stream/test/StreamMock.hpp"

TEST(LimitedOutputStreamTest, Insert)
{
    testing::StrictMock<infra::StreamWriterMock> writer;
    infra::LimitedStreamWriter limitedWriter(writer, 2);
    infra::StreamErrorPolicy errorPolicy(infra::softFail);

    std::array<uint8_t, 4> data{};
    EXPECT_CALL(writer, Insert(infra::Head(infra::MakeConst(infra::MakeRange(data)), 2), testing::Ref(errorPolicy)));
    limitedWriter.Insert(data, errorPolicy);
    EXPECT_TRUE(errorPolicy.Failed());
}

TEST(LimitedOutputStreamTest, Available_from_limit)
{
    testing::StrictMock<infra::StreamWriterMock> writer;
    infra::LimitedStreamWriter limitedWriter(writer, 2);
    infra::StreamErrorPolicy errorPolicy(infra::softFail);

    EXPECT_CALL(writer, Available()).WillOnce(testing::Return(3));
    EXPECT_EQ(2, limitedWriter.Available());
}

TEST(LimitedOutputStreamTest, Available_from_original_writer)
{
    testing::StrictMock<infra::StreamWriterMock> writer;
    infra::LimitedStreamWriter limitedWriter(writer, 2);
    infra::StreamErrorPolicy errorPolicy(infra::softFail);

    EXPECT_CALL(writer, Available()).WillOnce(testing::Return(1));
    EXPECT_EQ(1, limitedWriter.Available());
}
