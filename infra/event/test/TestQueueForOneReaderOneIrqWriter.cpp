#include "infra/event/QueueForOneReaderOneIrqWriter.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class QueueForOneReaderOneIrqWriterTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    infra::MockCallback<void()> callback;
    std::array<uint8_t, 5> buffer;
    infra::Optional<infra::QueueForOneReaderOneIrqWriter<uint8_t>> queue;
};

TEST_F(QueueForOneReaderOneIrqWriterTest, add_element)
{
    queue.emplace(buffer, [this]()
        {
            queue->Get();
            callback.callback();
        });

    queue->AddFromInterrupt(0);
    EXPECT_CALL(callback, callback());
    ExecuteAllActions();

    queue->AddFromInterrupt(1);
    EXPECT_CALL(callback, callback());
    ExecuteAllActions();
}

TEST_F(QueueForOneReaderOneIrqWriterTest, add_range)
{
    queue.emplace(buffer, [this]()
        {
            while (!queue->Empty())
                queue->Get();
            callback.callback();
        });

    std::array<uint8_t, 2> data = { { 0, 1 } };
    queue->AddFromInterrupt(data);
    EXPECT_CALL(callback, callback());
    ExecuteAllActions();
}

TEST_F(QueueForOneReaderOneIrqWriterTest, consume_1_before_get)
{
    queue.emplace(buffer, [this]() {});

    queue->AddFromInterrupt(0);
    queue->AddFromInterrupt(1);

    queue->Consume(1);
    EXPECT_EQ(1, queue->Get());
}

TEST_F(QueueForOneReaderOneIrqWriterTest, get_without_consume_using_array_operator)
{
    queue.emplace(buffer, [this]() {});

    std::array<uint8_t, 4> data = { { 0, 1, 2, 3 } };
    queue->AddFromInterrupt(data);

    EXPECT_EQ(0, (*queue)[0]);
    EXPECT_EQ(3, (*queue)[3]);

    queue->Consume(4);
    queue->AddFromInterrupt(data);

    EXPECT_EQ(0, (*queue)[0]);
    EXPECT_EQ(3, (*queue)[3]);
}

TEST_F(QueueForOneReaderOneIrqWriterTest, get_ContiguousRange)
{
    queue.emplace(buffer, [this]() {});

    EXPECT_TRUE(queue->ContiguousRange().empty());

    std::array<uint8_t, 2> data = { { 0, 1 } };
    queue->AddFromInterrupt(data);

    auto range = queue->ContiguousRange();
    EXPECT_EQ((std::vector<uint8_t>(data.begin(), data.end())), (std::vector<uint8_t>{ range.begin(), range.end() }));
}

TEST_F(QueueForOneReaderOneIrqWriterTest, get_ContiguousRange_while_queue_is_wrapped)
{
    queue.emplace(buffer, [this]() {});

    std::array<uint8_t, 3> data = { { 0, 1, 2 } };
    queue->AddFromInterrupt(data);
    infra::ConstByteRange range = queue->ContiguousRange();
    EXPECT_EQ((std::vector<uint8_t>{ 0, 1, 2 }), (std::vector<uint8_t>{ range.begin(), range.end() }));

    queue->Consume(3);
    queue->AddFromInterrupt(data);
    range = queue->ContiguousRange();
    EXPECT_EQ((std::vector<uint8_t>{ 0, 1 }), (std::vector<uint8_t>{ range.begin(), range.end() }));

    queue->Consume(range.size());
    range = queue->ContiguousRange();
    EXPECT_EQ((std::vector<uint8_t>{ 2 }), (std::vector<uint8_t>{ range.begin(), range.end() }));
}

TEST_F(QueueForOneReaderOneIrqWriterTest, get_ContiguousRange_with_offset)
{
    queue.emplace(buffer, [this]() {});

    std::array<uint8_t, 3> data = { { 0, 1, 2 } };
    queue->AddFromInterrupt(data);

    infra::ConstByteRange range = queue->ContiguousRange(1);
    EXPECT_EQ((std::vector<uint8_t>{ 1, 2 }), (std::vector<uint8_t>{ range.begin(), range.end() }));

    queue->Consume(1);
    range = queue->ContiguousRange();
    EXPECT_EQ((std::vector<uint8_t>{ 1, 2 }), (std::vector<uint8_t>{ range.begin(), range.end() }));
}

TEST_F(QueueForOneReaderOneIrqWriterTest, get_ContiguousRange_with_offset_while_queue_is_wrapped)
{
    queue.emplace(buffer, [this]() {});

    std::array<uint8_t, 3> data = { { 0, 1, 2 } };
    queue->AddFromInterrupt(data);
    infra::ConstByteRange range = queue->ContiguousRange(2);
    EXPECT_EQ((std::vector<uint8_t>{ 2 }), (std::vector<uint8_t>{ range.begin(), range.end() }));

    queue->Consume(3);

    queue->AddFromInterrupt(data);
    range = queue->ContiguousRange(0);
    EXPECT_EQ((std::vector<uint8_t>{ 0, 1 }), (std::vector<uint8_t>{ range.begin(), range.end() }));

    range = queue->ContiguousRange(2);
    EXPECT_EQ((std::vector<uint8_t>{ 2 }), (std::vector<uint8_t>{ range.begin(), range.end() }));
}

TEST_F(QueueForOneReaderOneIrqWriterTest, Size)
{
    queue.emplace(buffer, [this]() {});
    std::array<uint8_t, 4> full = { { 3, 1, 2, 4 } };
    std::array<uint8_t, 1> data1 = { { 7 } };
    std::array<uint8_t, 2> data2 = { { 5, 1 } };

    EXPECT_TRUE(queue->Empty());
    EXPECT_FALSE(queue->Full());
    EXPECT_EQ(0, queue->Size());

    queue->AddFromInterrupt(full);
    EXPECT_FALSE(queue->Empty());
    EXPECT_TRUE(queue->Full());
    EXPECT_EQ(4, queue->Size());

    queue->Consume(2);
    EXPECT_EQ(2, queue->Size());
    queue->AddFromInterrupt(data1);
    EXPECT_EQ(3, queue->Size());

    queue->Consume(1);
    EXPECT_EQ(2, queue->Size());
    queue->AddFromInterrupt(data2);
    EXPECT_EQ(4, queue->Size());
}

TEST_F(QueueForOneReaderOneIrqWriterTest, EmptySize)
{
    queue.emplace(buffer, [this]() {});
    EXPECT_EQ(sizeof(buffer) - 1, queue->EmptySize());
}

TEST_F(QueueForOneReaderOneIrqWriterTest, StreamReader)
{
    queue.emplace(buffer, [this]() {});
    std::array<uint8_t, 4> full = { { 1, 2, 3, 4 } };
    queue->AddFromInterrupt(full);

    infra::QueueForOneReaderOneIrqWriter<uint8_t>::StreamReader reader(*queue);
    infra::DataInputStream::WithErrorPolicy stream(reader, infra::noFail);

    EXPECT_EQ(4, stream.Available());
    EXPECT_FALSE(stream.Empty());

    EXPECT_EQ(1, stream.Extract<uint8_t>());
    EXPECT_EQ(2, reader.Peek(stream.ErrorPolicy()));

    EXPECT_EQ(3, stream.PeekContiguousRange(1).front());
    EXPECT_EQ(2, stream.PeekContiguousRange(1).size());
    EXPECT_EQ(4, stream.ContiguousRange().back());

    ASSERT_TRUE(queue->Full());
    reader.Commit();
    EXPECT_FALSE(queue->Full());

    ASSERT_FALSE(stream.Failed());
    reader.Peek(stream.ErrorPolicy());
    EXPECT_TRUE(stream.Failed());
}
