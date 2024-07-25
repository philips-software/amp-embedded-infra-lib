#include "infra/stream/AtomicByteQueue.hpp"
#include "infra/util/ConstructBin.hpp"
#include "gtest/gtest.h"

class AtomicByteQueueTest
    : public testing::Test
{
public:
    infra::AtomicByteQueue::WithStorage<4> deque;
    infra::AtomicByteQueueReader reader{ deque };
    infra::AtomicByteQueueWriter writer{ deque };
    infra::StreamErrorPolicy errorPolicy;
    infra::StreamErrorPolicy softErrorPolicy{ infra::softFail };
};

TEST_F(AtomicByteQueueTest, construction)
{
    EXPECT_EQ(0, deque.Size());
    EXPECT_EQ(4, deque.MaxSize());
    EXPECT_TRUE(deque.Empty());
    EXPECT_TRUE(deque.PeekContiguousRange(0).empty());

    EXPECT_TRUE(reader.Empty());
    EXPECT_EQ(0, reader.Available());

    EXPECT_EQ(4, writer.Available());
}

TEST_F(AtomicByteQueueTest, Insert_and_Extract)
{
    std::vector<uint8_t> data{ 1, 2, 3, 4 };
    writer.Insert(infra::MakeRange(data), errorPolicy);

    EXPECT_EQ(0, writer.Available());
    EXPECT_EQ(4, reader.Available());

    EXPECT_TRUE(infra::ContentsEqual(infra::MakeRange(data), reader.ExtractContiguousRange(10)));
}

TEST_F(AtomicByteQueueTest, Insert_overflows)
{
    std::vector<uint8_t> data{ 1, 2, 3, 4, 5 };
    writer.Insert(infra::MakeRange(data), softErrorPolicy);
    EXPECT_TRUE(softErrorPolicy.Failed());
    EXPECT_TRUE(infra::ContentsEqual(infra::Head(infra::MakeRange(data), 4), reader.ExtractContiguousRange(10)));
}

TEST_F(AtomicByteQueueTest, Rewind)
{
    std::vector<uint8_t> data{ 1, 2, 3, 4 };
    writer.Insert(infra::MakeRange(data), errorPolicy);

    reader.ExtractContiguousRange(2);
    auto marker = reader.ConstructSaveMarker();
    EXPECT_EQ(3, reader.ExtractContiguousRange(10).front());
    reader.Rewind(marker);
    EXPECT_EQ(3, reader.ExtractContiguousRange(10).front());
}

TEST_F(AtomicByteQueueTest, Peek)
{
    EXPECT_EQ(0, reader.Peek(softErrorPolicy));
    EXPECT_TRUE(softErrorPolicy.Failed());

    std::vector<uint8_t> data{ 1 };
    writer.Insert(infra::MakeRange(data), errorPolicy);

    EXPECT_EQ(1, reader.Peek(errorPolicy));
    EXPECT_FALSE(errorPolicy.Failed());
}

TEST_F(AtomicByteQueueTest, Extract)
{
    std::vector<uint8_t> data1{ 1, 2 };
    std::vector<uint8_t> data2{ 3, 4 };
    std::vector<uint8_t> data3{ 5, 6 };

    writer.Insert(infra::MakeRange(data1), errorPolicy);
    writer.Insert(infra::MakeRange(data2), errorPolicy);

    std::array<uint8_t, 2> r1;
    reader.Extract(infra::MakeRange(r1), errorPolicy);
    EXPECT_TRUE(infra::ContentsEqual(infra::ConstructBin()({ 1, 2 }).Range(), infra::MakeRange(r1)));
    reader.Commit();
    writer.Insert(infra::MakeRange(data3), errorPolicy);
    EXPECT_EQ(4, reader.Available());

    std::array<uint8_t, 4> r2;
    reader.Extract(infra::MakeRange(r2), errorPolicy);
    EXPECT_TRUE(infra::ContentsEqual(infra::ConstructBin()({ 3, 4, 5, 6 }).Range(), infra::MakeRange(r2)));
    reader.Commit();
}

TEST_F(AtomicByteQueueTest, PeekContiguousRange)
{
    std::vector<uint8_t> data1{ 1, 2, 3, 4 };
    std::vector<uint8_t> data2{ 5, 6 };

    writer.Insert(infra::MakeRange(data1), errorPolicy);

    EXPECT_TRUE(infra::ContentsEqual(infra::ConstructBin()({ 1, 2, 3, 4 }).Range(), reader.PeekContiguousRange(0)));
    reader.ExtractContiguousRange(2);
    reader.Commit();
    writer.Insert(infra::MakeRange(data2), errorPolicy);
    EXPECT_EQ(4, reader.Available());

    EXPECT_TRUE(infra::ContentsEqual(infra::ConstructBin()({ 3, 4, 5 }).Range(), reader.PeekContiguousRange(0)));
    EXPECT_TRUE(infra::ContentsEqual(infra::ConstructBin()({ 6 }).Range(), reader.PeekContiguousRange(3)));
}

TEST_F(AtomicByteQueueTest, Extract_overflows)
{
    std::vector<uint8_t> data1{ 1, 2 };

    writer.Insert(infra::MakeRange(data1), errorPolicy);

    std::array<uint8_t, 4> r1{ 0, 0, 0, 0 };
    reader.Extract(infra::MakeRange(r1), softErrorPolicy);

    EXPECT_TRUE(softErrorPolicy.Failed());
    EXPECT_TRUE(infra::ContentsEqual(infra::ConstructBin()({ 1, 2, 0, 0 }).Range(), infra::MakeRange(r1)));
}
