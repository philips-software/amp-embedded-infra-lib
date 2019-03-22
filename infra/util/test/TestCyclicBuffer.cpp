#include "gtest/gtest.h"
#include "infra/util/CyclicBuffer.hpp"

class CyclicBufferTest
    : public testing::Test
{
public:
    infra::CyclicBuffer<uint8_t>::WithStorage<4> buffer;
};

TEST_F(CyclicBufferTest, CreateEmpty)
{
    EXPECT_TRUE(buffer.Empty());
    EXPECT_FALSE(buffer.Full());
    EXPECT_EQ(0, buffer.Size());
    EXPECT_EQ(4, buffer.Available());
}

TEST_F(CyclicBufferTest, PushOne)
{
    buffer.Push(5);
    EXPECT_FALSE(buffer.Empty());
    EXPECT_FALSE(buffer.Full());
    EXPECT_EQ(1, buffer.Size());
    EXPECT_EQ(3, buffer.Available());
    EXPECT_EQ(std::vector<uint8_t>{ 5 }, buffer.ContiguousRange());
}

TEST_F(CyclicBufferTest, PushRange)
{
    buffer.Push(std::vector<uint8_t>{ 3, 7 });

    EXPECT_FALSE(buffer.Empty());
    EXPECT_FALSE(buffer.Full());
    EXPECT_EQ((std::vector<uint8_t>{ 3, 7 }), buffer.ContiguousRange());
}

TEST_F(CyclicBufferTest, PushEmptyRange)
{
    buffer.Push(std::vector<uint8_t>{ });

    EXPECT_TRUE(buffer.Empty());
    EXPECT_FALSE(buffer.Full());
    EXPECT_EQ((std::vector<uint8_t>{ }), buffer.ContiguousRange());
}

TEST_F(CyclicBufferTest, PushFull)
{
    buffer.Push(std::vector<uint8_t>{ 2, 3, 7, 11 });

    EXPECT_TRUE(buffer.Full());
    EXPECT_EQ((std::vector<uint8_t>{ 2, 3, 7, 11 }), buffer.ContiguousRange());

    infra::CyclicBuffer<uint8_t>::WithStorage<4> buffer2;
    buffer2.Push(std::vector<uint8_t>{ 2, 3, 7 });
    buffer2.Push(11);

    EXPECT_TRUE(buffer2.Full());
    EXPECT_EQ((std::vector<uint8_t>{ 2, 3, 7, 11 }), buffer2.ContiguousRange());
}

TEST_F(CyclicBufferTest, Pop)
{
    buffer.Push(5);
    buffer.Pop(1);

    EXPECT_TRUE(buffer.Empty());
}

TEST_F(CyclicBufferTest, PopNothing)
{
    buffer.Push(5);
    buffer.Pop(0);

    EXPECT_FALSE(buffer.Empty());
}

TEST_F(CyclicBufferTest, PushBeyondEnd)
{
    buffer.Push(std::vector<uint8_t>{ 3, 7, 9 });
    buffer.Pop(2);
    buffer.Push(std::vector<uint8_t>{ 2, 3 });

    EXPECT_EQ((std::vector<uint8_t>{ 9, 2 }), buffer.ContiguousRange());
    EXPECT_EQ(3, buffer.Size());
    EXPECT_EQ(1, buffer.Available());
}

TEST_F(CyclicBufferTest, PushFullBeyondEnd)
{
    buffer.Push(std::vector<uint8_t>{ 3, 2 });
    buffer.Pop(1);
    buffer.Push(std::vector<uint8_t>{ 5, 7, 9 });

    EXPECT_EQ((std::vector<uint8_t>{ 2, 5, 7 }), buffer.ContiguousRange());
    EXPECT_EQ(4, buffer.Size());
    EXPECT_EQ(0, buffer.Available());
}

TEST_F(CyclicBufferTest, PopUntilEnd)
{
    buffer.Push(std::vector<uint8_t>{ 3, 5, 9 });
    buffer.Pop(2);
    buffer.Push(std::vector<uint8_t>{ 2, 3, 7 });

    buffer.Pop(2);
    EXPECT_EQ((std::vector<uint8_t>{ 3, 7 }), buffer.ContiguousRange());
}

TEST_F(CyclicBufferTest, PopBeyondEnd)
{
    buffer.Push(std::vector<uint8_t>{ 3, 5, 9 });
    buffer.Pop(2);
    buffer.Push(std::vector<uint8_t>{ 2, 3, 7 });

    buffer.Pop(3);
    EXPECT_EQ((std::vector<uint8_t>{ 7 }), buffer.ContiguousRange());
}
