#include "infra/stream/BufferingStreamReader.hpp"
#include "infra/stream/test/StreamMock.hpp"
#include "gmock/gmock.h"

class BufferingStreamReaderTest
    : public testing::Test
{
public:
    ~BufferingStreamReaderTest()
    {
        EXPECT_CALL(input, Empty()).WillOnce(testing::Return(true));
    }

    void Extract(std::size_t amount)
    {
        std::vector<uint8_t> data(amount, 0);
        std::vector<uint8_t> inputData;
        EXPECT_CALL(input, ExtractContiguousRange(testing::_)).Times(testing::AnyNumber()).WillRepeatedly(testing::Invoke([&](std::size_t max)
            {
                inputData.resize(max);
                return infra::MakeRange(inputData);
            }));
        reader.Extract(infra::MakeRange(data), errorPolicy);
    }

    void ExpectBuffer(const std::vector<uint8_t> contents)
    {
        EXPECT_EQ(infra::BoundedDeque<uint8_t>::WithMaxSize<100>(contents.begin(), contents.end()), buffer);
    }

    infra::BoundedDeque<uint8_t>::WithMaxSize<4> buffer{ std::initializer_list<uint8_t>{ 1, 2 } };
    testing::StrictMock<infra::StreamReaderWithRewindingMock> input;
    infra::StreamErrorPolicy errorPolicy{ infra::noFail };
    infra::BufferingStreamReader reader{ buffer, input };
};

TEST_F(BufferingStreamReaderTest, Extract_from_empty_buffer)
{
    buffer.clear();
    std::array<uint8_t, 2> data;
    EXPECT_CALL(input, ExtractContiguousRange(2)).WillOnce(testing::Return(infra::ConstByteRange()));
    reader.Extract(data, errorPolicy);
    EXPECT_TRUE(errorPolicy.Failed());
}

TEST_F(BufferingStreamReaderTest, Extract_from_buffer)
{
    std::array<uint8_t, 2> data;
    reader.Extract(data, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 2>{ 1, 2 }), data);
    EXPECT_FALSE(errorPolicy.Failed());
}

TEST_F(BufferingStreamReaderTest, Extract_from_wrapped_buffer)
{
    buffer.push_back(3);
    buffer.push_back(4);
    buffer.pop_front();
    buffer.pop_front();
    buffer.push_back(5);
    buffer.push_back(6);

    std::array<uint8_t, 4> data;
    reader.Extract(data, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 4>{ 3, 4, 5, 6 }), data);
}

TEST_F(BufferingStreamReaderTest, Extract_from_buffer_and_input)
{
    std::array<uint8_t, 4> data;
    std::array<uint8_t, 2> inputData{ 3, 4 };
    EXPECT_CALL(input, ExtractContiguousRange(2)).WillOnce(testing::Invoke([&](std::size_t max)
        {
            return infra::MakeRange(inputData);
        }));
    reader.Extract(data, errorPolicy);
    EXPECT_EQ((std::array<uint8_t, 4>{ 1, 2, 3, 4 }), data);
}

TEST_F(BufferingStreamReaderTest, PeekContiguous_range_from_buffer_head)
{
    EXPECT_EQ((std::array<uint8_t, 2>{ 1, 2 }), reader.PeekContiguousRange(0));
}

TEST_F(BufferingStreamReaderTest, PeekContiguous_range_from_buffer_1)
{
    EXPECT_EQ((std::array<uint8_t, 1>{ 2 }), reader.PeekContiguousRange(1));
}

TEST_F(BufferingStreamReaderTest, PeekContiguous_range_from_input)
{
    std::array<uint8_t, 2> inputData{ 3, 4 };
    EXPECT_CALL(input, PeekContiguousRange(0)).WillOnce(testing::Invoke([&](std::size_t start)
        {
            return infra::MakeRange(inputData);
        }));
    EXPECT_EQ((std::array<uint8_t, 2>{ 3, 4 }), reader.PeekContiguousRange(2));
}

TEST_F(BufferingStreamReaderTest, Peek_from_buffer)
{
    EXPECT_EQ(1, reader.Peek(errorPolicy));
    reader.ExtractContiguousRange(2);

    std::array<uint8_t, 2> inputData{ 3, 4 };
    EXPECT_CALL(input, PeekContiguousRange(0)).WillOnce(testing::Invoke([&](std::size_t start)
        {
            return infra::MakeRange(inputData);
        }));
    EXPECT_EQ(3, reader.Peek(errorPolicy));
}

TEST_F(BufferingStreamReaderTest, Available)
{
    EXPECT_CALL(input, Available()).WillOnce(testing::Return(1));
    EXPECT_EQ(3, reader.Available());

    EXPECT_CALL(input, Available()).WillOnce(testing::Return(1));
    EXPECT_FALSE(reader.Empty());
}

TEST_F(BufferingStreamReaderTest, ConstructSaveMarker)
{
    EXPECT_EQ(0, reader.ConstructSaveMarker());

    reader.ExtractContiguousRange(2);
    EXPECT_EQ(2, reader.ConstructSaveMarker());
}

TEST_F(BufferingStreamReaderTest, Rewind_in_buffer)
{
    reader.ExtractContiguousRange(2);
    reader.Rewind(1);
    EXPECT_EQ(2, reader.Peek(errorPolicy));
}

TEST_F(BufferingStreamReaderTest, Rewind_from_input_to_input)
{
    Extract(4);

    EXPECT_CALL(input, ConstructSaveMarker()).WillOnce(testing::Return(100));
    EXPECT_CALL(input, Rewind(99));
    reader.Rewind(3);
    std::array<uint8_t, 1> inputData2{ 4 };
    EXPECT_CALL(input, PeekContiguousRange(1)).WillOnce(testing::Invoke([&](std::size_t start)
        {
            return infra::MakeRange(inputData2);
        }));
    EXPECT_EQ(4, reader.Peek(errorPolicy));
}

TEST_F(BufferingStreamReaderTest, Rewind_from_input_to_buffer)
{
    Extract(4);

    EXPECT_CALL(input, ConstructSaveMarker()).WillOnce(testing::Return(100));
    EXPECT_CALL(input, Rewind(98));
    reader.Rewind(1);
    EXPECT_EQ(2, reader.Peek(errorPolicy));
}

TEST_F(BufferingStreamReaderTest, destruction_reduces_buffer)
{
    Extract(1);

    EXPECT_CALL(input, Empty()).WillOnce(testing::Return(true));
    infra::ReConstruct(reader, buffer, input);

    ExpectBuffer({ 2 });
}

TEST_F(BufferingStreamReaderTest, destruction_consumes_buffer)
{
    Extract(3);

    EXPECT_CALL(input, Empty()).WillOnce(testing::Return(true));
    infra::ReConstruct(reader, buffer, input);

    ExpectBuffer({});
}

TEST_F(BufferingStreamReaderTest, destruction_stores_input)
{
    EXPECT_CALL(input, Empty()).WillOnce(testing::Return(false)).WillOnce(testing::Return(true));
    std::array<uint8_t, 2> inputData{ 3, 4 };
    EXPECT_CALL(input, ExtractContiguousRange(std::numeric_limits<std::size_t>::max())).WillOnce(testing::Invoke([&](std::size_t max)
        {
            return infra::MakeRange(inputData);
        }));
    infra::ReConstruct(reader, buffer, input);

    ExpectBuffer({ 1, 2, 3, 4 });
}

TEST_F(BufferingStreamReaderTest, destruction_stores_input_until_empty)
{
    EXPECT_CALL(input, Empty()).WillOnce(testing::Return(false)).WillOnce(testing::Return(false)).WillOnce(testing::Return(true));
    std::array<uint8_t, 1> inputData{ 3 };
    EXPECT_CALL(input, ExtractContiguousRange(std::numeric_limits<std::size_t>::max())).WillRepeatedly(testing::Invoke([&](std::size_t max)
        {
            return infra::MakeRange(inputData);
        }));
    infra::ReConstruct(reader, buffer, input);

    ExpectBuffer({ 1, 2, 3, 3 });
}
