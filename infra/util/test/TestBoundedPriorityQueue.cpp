#include "infra/util/BoundedPriorityQueue.hpp"
#include "infra/util/test_helper/MoveConstructible.hpp"
#include "gtest/gtest.h"

TEST(BoundedPriorityQueueTest, TestConstructedEmpty)
{
    infra::BoundedPriorityQueue<int, 5> queue;

    EXPECT_TRUE(queue.empty());
    EXPECT_FALSE(queue.full());
    EXPECT_EQ(0, queue.size());
}

TEST(BoundedPriorityQueueTest, TestConstructionWithRange)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedPriorityQueue<int, 5> queue(range, range + 3);

    EXPECT_EQ(3, queue.size());

    EXPECT_EQ(2, *queue.begin());
    EXPECT_EQ(1, *(queue.begin() + 1));
    EXPECT_EQ(0, *(queue.begin() + 2));
}

TEST(BoundedPriorityQueueTest, TestCopyConstruction)
{
    infra::BoundedPriorityQueue<int, 5> original;
    original.push(4);
    infra::BoundedPriorityQueue<int, 5> copy(original);

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(4, copy.top());
}

TEST(BoundedPriorityQueueTest, TestMoveConstruction)
{
    infra::BoundedPriorityQueue<infra::MoveConstructible, 5> original;
    original.emplace(2);
    infra::BoundedPriorityQueue<infra::MoveConstructible, 5> copy(std::move(original));

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy[0].x);
}

TEST(BoundedPriorityQueueTest, TestAssignment)
{
    infra::BoundedPriorityQueue<int, 5> original;
    original.push(4);
    infra::BoundedPriorityQueue<int, 5> copy;
    copy = original;

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(4, copy[0]);
}

TEST(BoundedPriorityQueueTest, TestMove)
{
    infra::BoundedPriorityQueue<infra::MoveConstructible, 5> original;
    original.emplace(2);
    infra::BoundedPriorityQueue<infra::MoveConstructible, 5> copy;
    copy = std::move(original);

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy[0].x);
}

TEST(BoundedPriorityQueueTest, TestPop)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedPriorityQueue<int, 5> queue(range, range + 3);

    queue.pop();
    EXPECT_EQ(2, queue.size());
    EXPECT_EQ(1, queue[0]);
    EXPECT_EQ(0, queue[1]);
}

TEST(BoundedPriorityQueueTest, TestErase)
{
    int range[7] = { 0, 1, 2, 3, 4, 5, 6 };
    infra::BoundedPriorityQueue<int, 10> queue(range, range + 7);

    queue.erase(queue.begin() + 1);
    EXPECT_EQ(6, queue.size());
    EXPECT_EQ(6, queue[0]);
    EXPECT_EQ(3, queue[1]);
    EXPECT_EQ(5, queue[2]);
    EXPECT_EQ(2, queue[3]);
    EXPECT_EQ(1, queue[4]);
    EXPECT_EQ(0, queue[5]);
}

TEST(BoundedPriorityQueueTest, TestErase2)
{
    int range[7] = { 0, 3, 2, 1, 4, 5, 6 };
    infra::BoundedPriorityQueue<int, 10> queue(range, range + 7);

    queue.erase(queue.begin() + 1);
    EXPECT_EQ(6, queue.size());
    EXPECT_EQ(6, queue[0]);
    EXPECT_EQ(3, queue[1]);
    EXPECT_EQ(5, queue[2]);
    EXPECT_EQ(1, queue[3]);
    EXPECT_EQ(2, queue[4]);
    EXPECT_EQ(0, queue[5]);
}

TEST(BoundedPriorityQueueTest, TestEraseLastElement)
{
    infra::BoundedPriorityQueue<int, 10> queue;
    queue.push(1);

    queue.erase(queue.begin());
    EXPECT_EQ(0, queue.size());
}

TEST(BoundedPriorityQueueTest, TestBeginAndEnd)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedPriorityQueue<int, 5> queue(range, range + 3);

    EXPECT_EQ(queue.end(), queue.begin() + 3);
    EXPECT_EQ(queue.cbegin(), queue.begin());
    EXPECT_EQ(queue.cend(), queue.end());
    EXPECT_EQ(queue.rend(), queue.rbegin() + 3);

    EXPECT_EQ(2, *queue.begin());
    EXPECT_EQ(0, *queue.rbegin());
}

TEST(BoundedPriorityQueueTest, TestFull)
{
    infra::BoundedPriorityQueue<int, 1> queue;
    queue.push(5);

    EXPECT_TRUE(queue.full());
}

TEST(BoundedPriorityQueueTest, TestSwap)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedPriorityQueue<int, 5> queue1(range1, range1 + 3);
    int range2[3] = { 3, 4, 5 };
    infra::BoundedPriorityQueue<int, 5> queue2(range2, range2 + 3);

    swap(queue1, queue2);

    EXPECT_EQ(3, queue1.size());
    EXPECT_EQ(5, queue1.top());
    EXPECT_EQ(3, queue2.size());
    EXPECT_EQ(2, queue2.top());
}

TEST(BoundedPriorityQueueTest, TestSwapDifferentSizes)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedPriorityQueue<int, 5> queue1(range1, range1 + 3);
    int range2[2] = { 3, 4 };
    infra::BoundedPriorityQueue<int, 5> queue2(range2, range2 + 2);

    swap(queue1, queue2);

    EXPECT_EQ(2, queue1.size());
    EXPECT_EQ(4, queue1.top());
    EXPECT_EQ(3, queue2.size());
    EXPECT_EQ(2, queue2.top());
}

TEST(BoundedPriorityQueueTest, TestClear)
{
    infra::BoundedPriorityQueue<int, 5> queue;
    queue.push(5);
    queue.clear();

    EXPECT_EQ(0, queue.size());
    EXPECT_TRUE(queue.empty());
}
