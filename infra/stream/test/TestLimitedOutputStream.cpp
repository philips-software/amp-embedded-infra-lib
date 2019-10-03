#include "gmock/gmock.h"
#include "infra/stream/LimitedOutputStream.hpp"
#include "infra/stream/OverwriteStream.hpp"
#include "infra/stream/SavedMarkerStream.hpp"
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

TEST(LimitedOutputStreamTest, SavedState)
{
    testing::StrictMock<infra::StreamWriterMock> writer;
    infra::LimitedStreamWriter limitedWriter(writer, 4);
    infra::DataOutputStream::WithErrorPolicy stream(limitedWriter);

    uint8_t data;
    EXPECT_CALL(writer, Insert(infra::MakeConstByteRange(data), testing::_));
    stream << data;

    EXPECT_CALL(writer, ConstructSaveMarker()).WillOnce(testing::Return(1));
    auto save = limitedWriter.ConstructSaveMarker();

    EXPECT_CALL(writer, Insert(infra::MakeConstByteRange(data), testing::_));
    stream << data;

    {
        uint16_t state;
        EXPECT_CALL(writer, SaveState(1)).WillOnce(testing::Return(infra::MakeByteRange(state)));
        infra::SavedMarkerDataStream savedStream(stream, save);
        EXPECT_CALL(writer, Available()).WillOnce(testing::Return(10));
        EXPECT_EQ(2, limitedWriter.Available());

        EXPECT_CALL(writer, Insert(infra::MakeConstByteRange(data), testing::_));
        stream << data;

        EXPECT_CALL(writer, RestoreState(infra::MakeByteRange(state)));
    }
}

TEST(LimitedOutputStreamTest, Overwrite)
{
    testing::StrictMock<infra::StreamWriterMock> writer;
    infra::LimitedStreamWriter limitedWriter(writer, 4);
    infra::DataOutputStream::WithErrorPolicy stream(limitedWriter);

    uint8_t data;
    EXPECT_CALL(writer, Insert(infra::MakeConstByteRange(data), testing::_));
    stream << data;

    EXPECT_CALL(writer, ConstructSaveMarker()).WillOnce(testing::Return(1));
    auto save = limitedWriter.ConstructSaveMarker();

    EXPECT_CALL(writer, Insert(infra::MakeConstByteRange(data), testing::_));
    stream << data;

    {
        uint16_t state;
        EXPECT_CALL(writer, Overwrite(1)).WillOnce(testing::Return(infra::MakeByteRange(state)));
        EXPECT_CALL(writer, ConstructSaveMarker()).WillOnce(testing::Return(2));
        infra::OverwriteDataStream savedStream(stream, save);
        EXPECT_CALL(writer, Available()).WillOnce(testing::Return(10));
        EXPECT_EQ(3, limitedWriter.Available());

        EXPECT_CALL(writer, Insert(infra::MakeConstByteRange(data), testing::_));
        stream << data;
    }
}
