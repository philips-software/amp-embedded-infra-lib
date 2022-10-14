#include "gtest/gtest.h"
#include "infra/util/IntrusivePriorityQueue.hpp"

struct QueueInt
    : infra::IntrusivePriorityQueue<QueueInt>::NodeType
{
    QueueInt(int v)
        : value(v)
    {}

    int value;

    bool operator==(const QueueInt& other) const
    {
        return value == other.value;
    }

    bool operator<(const QueueInt& other) const
    {
        return value < other.value;
    }
};

TEST(IntrusivePriorityQueueTest, TestConstructedEmpty)
{
    infra::IntrusivePriorityQueue<QueueInt> queue;

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(0, queue.size());
}

TEST(IntrusivePriorityQueueTest, TestConstructionWithRange)
{
    QueueInt range[3] = { 0, 1, 2 };
    infra::IntrusivePriorityQueue<QueueInt> queue(range, range + 3);

    EXPECT_EQ(3, queue.size());

    EXPECT_EQ(QueueInt(2), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(1), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(0), queue.top());
}

TEST(IntrusivePriorityQueueTest, TestMove)
{
    infra::IntrusivePriorityQueue<QueueInt> original;
    QueueInt i4 = 4;
    original.push(i4);
    infra::IntrusivePriorityQueue<QueueInt> copy;
    copy = std::move(original);

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(QueueInt(4), copy.top());
}

TEST(IntrusivePriorityQueueTest, TestPop)
{
    QueueInt range[3] = { 0, 1, 2 };
    infra::IntrusivePriorityQueue<QueueInt> queue(range, range + 3);

    queue.pop();
    EXPECT_EQ(2, queue.size());
    EXPECT_EQ(QueueInt(1), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(0), queue.top());
}

TEST(IntrusivePriorityQueueTest, TestErase)
{
    QueueInt range[7] = { 0, 1, 2, 3, 4, 5, 6 };
    infra::IntrusivePriorityQueue<QueueInt> queue(range, range + 7);

    queue.erase(range[4]);
    EXPECT_EQ(queue.size(), 6);
    EXPECT_EQ(QueueInt(6), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(5), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(3), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(2), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(1), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(0), queue.top());
    queue.pop();
}

TEST(IntrusivePriorityQueueTest, TestErase2)
{
    QueueInt range[7] = { 0, 3, 2, 1, 4, 5, 6 };
    infra::IntrusivePriorityQueue<QueueInt> queue(range, range + 7);

    queue.erase(range[1]);
    EXPECT_EQ(queue.size(), 6);
    EXPECT_EQ(QueueInt(6), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(5), queue.top());
    queue.pop();
    QueueInt x = queue.top();
    EXPECT_EQ(QueueInt(4), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(2), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(1), queue.top());
    queue.pop();
    EXPECT_EQ(QueueInt(0), queue.top());
    queue.pop();
}

TEST(IntrusivePriorityQueueTest, TestEraseLastElement)
{
    infra::IntrusivePriorityQueue<QueueInt> queue;
    QueueInt i4 = 4;
    queue.push(i4);

    queue.erase(i4);
    EXPECT_EQ(0, queue.size());
}

TEST(IntrusivePriorityQueueTest, TestSwap)
{
    QueueInt range1[3] = { 0, 1, 2 };
    infra::IntrusivePriorityQueue<QueueInt> queue1(range1, range1 + 3);
    QueueInt range2[3] = { 3, 4, 5 };
    infra::IntrusivePriorityQueue<QueueInt> queue2(range2, range2 + 3);

    swap(queue1, queue2);

    infra::IntrusivePriorityQueue<QueueInt> expectedQueue1(range2, range2 + 3);
    infra::IntrusivePriorityQueue<QueueInt> expectedQueue2(range1, range1 + 3);
    EXPECT_EQ(QueueInt(5), queue1.top());
    EXPECT_EQ(QueueInt(2), queue2.top());
}

TEST(IntrusivePriorityQueueTest, TestClear)
{
    infra::IntrusivePriorityQueue<QueueInt> queue;
    QueueInt i4 = 4;
    queue.push(i4);
    queue.clear();

    infra::IntrusivePriorityQueue<QueueInt> expectedVector;
    EXPECT_EQ(0, queue.size());
}
