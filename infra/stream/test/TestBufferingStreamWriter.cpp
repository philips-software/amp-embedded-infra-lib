#include "infra/stream/BufferingStreamWriter.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "gmock/gmock.h"

class BufferingStreamWriterTest
    : public testing::Test
{
public:
    void ExpectBuffer(const std::vector<uint8_t> contents)
    {
        EXPECT_EQ(infra::BoundedDeque<uint8_t>::WithMaxSize<100>(contents.begin(), contents.end()), buffer);
    }

    infra::BoundedDeque<uint8_t>::WithMaxSize<4> buffer;
    testing::StrictMock<infra::StreamWriterMock> output;
    infra::StreamErrorPolicy errorPolicy{ infra::noFail };
    infra::Execute execute{ [this]()
        {
            EXPECT_CALL(output, Available()).WillRepeatedly(testing::Return(0));
            EXPECT_CALL(output, Insert(testing::_, testing::_)).Times(2);
        } };
    infra::BufferingStreamWriter writer{ buffer, output };
};

TEST_F(BufferingStreamWriterTest, Insert_into_output)
{
    std::array<uint8_t, 2> data{ 3, 4 };
    EXPECT_CALL(output, Available()).WillOnce(testing::Return(10));
    EXPECT_CALL(output, Insert(infra::ContentsEqual(data), testing::Ref(errorPolicy)));
    writer.Insert(data, errorPolicy);

    ExpectBuffer({});
    EXPECT_EQ(2, writer.ConstructSaveMarker());
}

TEST_F(BufferingStreamWriterTest, Insert_overflows_output)
{
    std::array<uint8_t, 2> data{ 3, 4 };
    EXPECT_CALL(output, Available()).WillOnce(testing::Return(1));
    EXPECT_CALL(output, Insert(infra::ByteRangeContentsEqual(infra::Head(infra::MakeRange(data), 1)), testing::Ref(errorPolicy)));
    writer.Insert(data, errorPolicy);

    ExpectBuffer({ 4 });
    EXPECT_EQ(2, writer.ConstructSaveMarker());
}

TEST_F(BufferingStreamWriterTest, GetProcessedBytesSince)
{
    EXPECT_EQ(0, writer.GetProcessedBytesSince(0));

    std::array<uint8_t, 2> data{ 3, 4 };
    EXPECT_CALL(output, Available()).WillOnce(testing::Return(10));
    EXPECT_CALL(output, Insert(infra::ContentsEqual(data), testing::Ref(errorPolicy)));
    writer.Insert(data, errorPolicy);

    EXPECT_EQ(2, writer.GetProcessedBytesSince(0));
    EXPECT_EQ(1, writer.GetProcessedBytesSince(1));
}

TEST_F(BufferingStreamWriterTest, LoadRemainder_loads_from_buffer)
{
    std::array<uint8_t, 2> data{ 1, 2 };
    buffer.insert(buffer.end(), data.begin(), data.end());

    EXPECT_CALL(output, Available()).WillOnce(testing::Return(2)).WillOnce(testing::Return(0));
    EXPECT_CALL(output, Insert(infra::ContentsEqual(data), testing::_));
    EXPECT_CALL(output, Insert(infra::ByteRangeContentsEqual(infra::ConstByteRange()), testing::_));
    infra::ReConstruct(writer, buffer, output);
}

TEST_F(BufferingStreamWriterTest, LoadRemainder_loads_from_circular_buffer)
{
    std::array<uint8_t, 2> data1{ 1, 2 };
    std::array<uint8_t, 2> data2{ 3, 4 };
    std::array<uint8_t, 2> data3{ 5, 6 };
    buffer.insert(buffer.end(), data1.begin(), data1.end());
    buffer.insert(buffer.end(), data2.begin(), data2.end());
    buffer.pop_front();
    buffer.pop_front();
    buffer.insert(buffer.end(), data3.begin(), data3.end());

    EXPECT_CALL(output, Available()).WillOnce(testing::Return(4)).WillOnce(testing::Return(2));
    EXPECT_CALL(output, Insert(infra::ContentsEqual(data2), testing::_));
    EXPECT_CALL(output, Insert(infra::ContentsEqual(data3), testing::_));
    infra::ReConstruct(writer, buffer, output);
}

TEST_F(BufferingStreamWriterTest, LoadRemainder_constrained_by_output_writes_in_chunks)
{
    std::array<uint8_t, 2> data{ 1, 2 };
    buffer.insert(buffer.end(), data.begin(), data.end());

    EXPECT_CALL(output, Available()).WillOnce(testing::Return(1)).WillOnce(testing::Return(0));
    EXPECT_CALL(output, Insert(infra::ByteRangeContentsEqual(infra::Head(infra::MakeRange(data), 1)), testing::_));
    EXPECT_CALL(output, Insert(infra::ByteRangeContentsEqual(infra::ConstByteRange()), testing::_));
    infra::ReConstruct(writer, buffer, output);

    EXPECT_CALL(output, Available()).WillOnce(testing::Return(1)).WillOnce(testing::Return(0));
    EXPECT_CALL(output, Insert(infra::ByteRangeContentsEqual(infra::DiscardHead(infra::MakeRange(data), 1)), testing::_));
    EXPECT_CALL(output, Insert(infra::ByteRangeContentsEqual(infra::ConstByteRange()), testing::_));
    infra::ReConstruct(writer, buffer, output);
}
