#include "infra/util/BoundedDeque.hpp"
#include "infra/util/test_helper/MoveConstructible.hpp"
#include "gtest/gtest.h"
#include <memory>

TEST(BoundedDequeTest, TestConstructedEmpty)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque;

    EXPECT_TRUE(deque.empty());
    EXPECT_FALSE(deque.full());
    EXPECT_EQ(0, deque.size());
    EXPECT_EQ(5, deque.max_size());
}

TEST(BoundedDequeTest, TestConstructionWith2Elements)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(2), 4);

    EXPECT_FALSE(deque.empty());
    EXPECT_FALSE(deque.full());
    EXPECT_EQ(2, deque.size());

    EXPECT_EQ(4, deque[0]);
    EXPECT_EQ(4, deque[1]);
}

TEST(BoundedDequeTest, TestConstructionWithRange)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(3, deque.size());

    EXPECT_EQ(0, deque[0]);
    EXPECT_EQ(1, deque[1]);
    EXPECT_EQ(2, deque[2]);
}

TEST(BoundedDequeTest, TestConstructionWithInitalizerList)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque({ 0, 1, 2 });

    EXPECT_EQ(3, deque.size());

    EXPECT_EQ(0, deque[0]);
    EXPECT_EQ(1, deque[1]);
    EXPECT_EQ(2, deque[2]);
}

TEST(BoundedDequeTest, TestCopyConstruction)
{
    infra::BoundedDeque<int>::WithMaxSize<5> original(std::size_t(2), 4);
    infra::BoundedDeque<int>::WithMaxSize<5> copy(original);

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(4, copy[0]);
    EXPECT_EQ(4, copy[1]);
}

TEST(BoundedDequeTest, TestCopyConstructionWrapped)
{
    infra::BoundedDeque<int>::WithMaxSize<5> original(std::size_t(1), 4);
    original.push_front(3);

    infra::BoundedDeque<int>::WithMaxSize<5> copy(original);

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(3, copy[0]);
    EXPECT_EQ(4, copy[1]);
}

TEST(BoundedDequeTest, TestMoveConstruction)
{
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_back(2);
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> copy(std::move(original));

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy[0].x);
}

TEST(BoundedDequeTest, TestMoveConstructionWrapped)
{
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_back(2);
    original.emplace_front(1);
    original.emplace_front(3);
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> copy(std::move(original));

    EXPECT_EQ(3, copy.size());
    EXPECT_EQ(3, copy[0].x);
    EXPECT_EQ(1, copy[1].x);
    EXPECT_EQ(2, copy[2].x);
}

TEST(BoundedDequeTest, TestAssignment)
{
    infra::BoundedDeque<int>::WithMaxSize<5> original(std::size_t(2), 4);
    infra::BoundedDeque<int>::WithMaxSize<5> copy;
    copy = original;

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(4, copy[0]);
}

TEST(BoundedDequeTest, TestSelfAssignment)
{
    infra::BoundedDeque<int>::WithMaxSize<5> original(std::size_t(2), 4);
    original = original;

    EXPECT_EQ(2, original.size());
    EXPECT_EQ(4, original[0]);
}

TEST(BoundedDequeTest, TestAssignmentWrapped)
{
    infra::BoundedDeque<int>::WithMaxSize<5> original(std::size_t(1), 4);
    original.push_front(3);
    infra::BoundedDeque<int>::WithMaxSize<5> copy;
    copy = original;

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(3, copy[0]);
    EXPECT_EQ(4, copy[1]);
}

TEST(BoundedDequeTest, TestAssignmentInitializerList)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque({ 1, 2, 3 });

    EXPECT_EQ(3, deque.size());
    EXPECT_EQ(1, deque[0]);
    EXPECT_EQ(2, deque[1]);
}

TEST(BoundedDequeTest, TestAssignmentInitializerListWrapped)
{
    infra::BoundedDeque<int>::WithMaxSize<5> dequeStorage(std::size_t(1), 4);
    infra::BoundedDeque<int>& deque = dequeStorage;
    deque.push_front(3);
    deque = { 1, 2 };

    EXPECT_EQ(2, deque.size());
    EXPECT_EQ(1, deque[0]);
    EXPECT_EQ(2, deque[1]);
}

TEST(BoundedDequeTest, TestMove)
{
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_back(2);
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> copy;
    copy = std::move(original);

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy[0].x);
}

TEST(BoundedDequeTest, TestMoveWrapped)
{
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_back(2);
    original.emplace_front(1);
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> copy;
    copy = std::move(original);

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(1, copy[0].x);
    EXPECT_EQ(2, copy[1].x);
}

TEST(BoundedDequeTest, TestBeginAndEnd)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(deque.end(), deque.begin() + 3);
    EXPECT_EQ(deque.cbegin(), deque.begin());
    EXPECT_EQ(deque.cend(), deque.end());
    EXPECT_EQ(deque.rend(), deque.rbegin() + 3);

    EXPECT_EQ(0, *deque.begin());
    EXPECT_EQ(2, *deque.rbegin());
    EXPECT_EQ(2, *deque.crbegin());
    EXPECT_EQ(2, *(deque.end() - 1));
    EXPECT_EQ(0, *(deque.rend() - 1));
    EXPECT_EQ(0, *(deque.crend() - 1));
}

TEST(BoundedDequeTest, TestBeginAndEndWrapped)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);
    deque.push_front(-1);

    EXPECT_EQ(deque.end(), deque.begin() + 4);
    EXPECT_EQ(deque.cbegin(), deque.begin());
    EXPECT_EQ(deque.cend(), deque.end());
    EXPECT_EQ(deque.rend(), deque.rbegin() + 4);

    EXPECT_EQ(-1, *deque.begin());
    EXPECT_EQ(2, *deque.rbegin());
    EXPECT_EQ(2, *(deque.end() - 1));
    EXPECT_EQ(-1, *(deque.rend() - 1));
}

TEST(BoundedDequeTest, TestContiguousRange)
{
    infra::BoundedDeque<int>::WithMaxSize<5> emptyDeque;
    EXPECT_EQ(0, emptyDeque.contiguous_range(emptyDeque.begin()).size());
    EXPECT_EQ(0, static_cast<const infra::BoundedDeque<int>&>(emptyDeque).contiguous_range(emptyDeque.begin()).size());

    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(3, deque.contiguous_range(deque.begin()).size());
    EXPECT_EQ(0, deque.contiguous_range(deque.begin()).front());

    deque.push_front(-1);
    deque.push_front(-2);

    EXPECT_EQ(2, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range(deque.begin()).size());
    EXPECT_EQ(-2, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range(deque.begin()).front());
    EXPECT_EQ(1, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range(deque.begin() + 1).size());
    EXPECT_EQ(-1, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range(deque.begin() + 1).front());
    EXPECT_EQ(3, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range(deque.begin() + 2).size());
    EXPECT_EQ(0, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range(deque.begin() + 2).front());
}

TEST(BoundedDequeTest, TestContiguousRangeAtEnd)
{
    infra::BoundedDeque<int>::WithMaxSize<5> emptyDeque;
    EXPECT_EQ(0, emptyDeque.contiguous_range_at_end(emptyDeque.begin()).size());
    EXPECT_EQ(0, static_cast<const infra::BoundedDeque<int>&>(emptyDeque).contiguous_range_at_end(emptyDeque.begin()).size());

    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(3, deque.contiguous_range_at_end(deque.end()).size());
    EXPECT_EQ(0, deque.contiguous_range_at_end(deque.end()).front());

    deque.push_front(-1);
    deque.push_front(-2);

    EXPECT_EQ(3, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range_at_end(deque.end()).size());
    EXPECT_EQ(0, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range_at_end(deque.end()).front());
    EXPECT_EQ(2, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range_at_end(deque.end() - 1).size());
    EXPECT_EQ(1, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range_at_end(deque.end() - 1).back());
    EXPECT_EQ(2, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range_at_end(deque.end() - 3).size());
    EXPECT_EQ(-2, static_cast<const infra::BoundedDeque<int>&>(deque).contiguous_range_at_end(deque.end() - 3).front());
}

TEST(BoundedDequeTest, TestResizeBigger)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(2), 4);
    deque.resize(3, 5);

    EXPECT_EQ(3, deque.size());
    EXPECT_EQ(4, deque[0]);
    EXPECT_EQ(5, deque[2]);
}

TEST(BoundedDequeTest, TestResizeSmaller)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(2), 4);
    deque.resize(1, 5);

    EXPECT_EQ(1, deque.size());
    EXPECT_EQ(4, deque[0]);
}

TEST(BoundedDequeTest, TestFull)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(5), 4);

    EXPECT_TRUE(deque.full());
}

TEST(BoundedDequeTest, TestFullWrapped)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(5), 4);
    deque.pop_front();
    deque.push_back(1);

    EXPECT_TRUE(deque.full());
}

TEST(BoundedDequeTest, TestFront)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(0, deque.front());
    EXPECT_EQ(0, const_cast<const infra::BoundedDeque<int>::WithMaxSize<5>&>(deque).front());
}

TEST(BoundedDequeTest, TestFrontWrapped)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);
    deque.push_front(-1);

    EXPECT_EQ(-1, deque.front());
}

TEST(BoundedDequeTest, TestBack)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(2, deque.back());
    EXPECT_EQ(2, const_cast<const infra::BoundedDeque<int>::WithMaxSize<5>&>(deque).back());
}

TEST(BoundedDequeTest, TestBackWrapped)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<3> deque(range, range + 3);
    deque.pop_front();
    deque.push_back(5);

    EXPECT_EQ(5, deque.back());
}

TEST(BoundedDequeTest, TestAssignRange)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    int otherRange[2] = { 4, 5 };
    deque.assign(otherRange, otherRange + 2);

    EXPECT_EQ(2, deque.size());
    EXPECT_EQ(4, deque[0]);
    EXPECT_EQ(5, deque[1]);
}

TEST(BoundedDequeTest, TestAssignN)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    deque.assign(std::size_t(2), 4);

    EXPECT_EQ(2, deque.size());
    EXPECT_EQ(4, deque[0]);
    EXPECT_EQ(4, deque[1]);
}

TEST(BoundedDequeTest, TestAssignInitializerList)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque({ 0, 1, 2 });
    deque.push_front(-1);
    deque.assign({ 4, 5, 6 });

    EXPECT_EQ(3, deque.size());
    EXPECT_EQ(4, deque[0]);
    EXPECT_EQ(5, deque[1]);
    EXPECT_EQ(6, deque[2]);
}

TEST(BoundedDequeTest, TestPushFront)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(2), 4);
    deque.push_front(1);

    EXPECT_EQ(3, deque.size());
    EXPECT_EQ(1, deque[0]);
}

TEST(BoundedDequeTest, TestPopFront)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(2), 4);
    deque.pop_front();

    EXPECT_EQ(1, deque.size());
    EXPECT_EQ(4, deque[0]);
}

TEST(BoundedDequeTest, TestPopWrapped)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(2), 4);
    deque.push_front(1);
    deque.push_front(0);

    deque.pop_front();

    EXPECT_EQ(3, deque.size());
    EXPECT_EQ(1, deque[0]);
}

TEST(BoundedDequeTest, TestPushBack)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque;
    deque.push_back(2);
    deque.push_back(3);

    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque;
    expectedDeque.push_back(2);
    expectedDeque.push_back(3);

    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestPushBackWrapped)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque;
    for (auto i = 0; i < 5; ++i)
        deque.push_back(1);

    deque.pop_front();
    deque.pop_front();
    deque.pop_front();
    deque.pop_front();

    deque.push_back(2);
    deque.push_back(3);

    EXPECT_EQ(1, deque[0]);
    EXPECT_EQ(2, deque[1]);
    EXPECT_EQ(3, deque[2]);
}

TEST(BoundedDequeTest, TestPopBack)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(2), 4);
    deque.pop_back();

    EXPECT_EQ(1, deque.size());
    EXPECT_EQ(4, deque[0]);
}

TEST(BoundedDequeTest, TestPopBackWrapped)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(1), 4);
    deque.push_front(1);

    deque.pop_back();

    EXPECT_EQ(1, deque.size());
    EXPECT_EQ(1, deque[0]);
}

TEST(BoundedDequeTest, TestInsertOneLvalueInTheMiddle)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    int i = 5;
    EXPECT_EQ(5, *deque.insert(deque.begin() + 1, i));

    int expectedRange[4] = { 0, 5, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 4);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestInsertOneRvalueInTheMiddle)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(5, *deque.insert(deque.begin() + 1, 5));

    int expectedRange[4] = { 0, 5, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 4);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestInsertOneAtTheEnd)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(5, *deque.insert(deque.end(), 5));

    int expectedRange[4] = { 0, 1, 2, 5 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 4);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestInsertManyInTheMiddle)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(5, *deque.insert(deque.begin() + 2, std::size_t(2), 5));

    int expectedRange[5] = { 0, 1, 5, 5, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 5);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestInsertManyInTheMiddleWrapped)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque;
    deque.push_front(1);
    deque.push_front(0);
    deque.push_back(2);

    EXPECT_EQ(5, *deque.insert(deque.begin() + 1, std::size_t(2), 5));

    int expectedRange[5] = { 0, 5, 5, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 5);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestInsertManyAtTheFront)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(5, *deque.insert(deque.begin(), std::size_t(2), 5));

    int expectedRange[5] = { 5, 5, 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 5);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestInsertRange)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    int insertRange[2] = { 5, 6 };
    EXPECT_EQ(5, *deque.insert(deque.begin() + 2, insertRange, insertRange + 2));

    int expectedRange[5] = { 0, 1, 5, 6, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 5);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestInsertInitializerList)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    EXPECT_EQ(5, *deque.insert(deque.begin() + 2, { 5, 5 }));

    int expectedRange[5] = { 0, 1, 5, 5, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 5);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestInsertMove)
{
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> deque;
    deque.emplace_back(0);
    deque.emplace_back(1);
    deque.emplace_back(2);

    EXPECT_EQ(5, deque.insert(deque.begin() + 1, infra::MoveConstructible(5))->x);

    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> expectedDeque;
    expectedDeque.emplace_back(0);
    expectedDeque.emplace_back(5);
    expectedDeque.emplace_back(1);
    expectedDeque.emplace_back(2);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestEraseOne)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    deque.erase(deque.begin() + 1);

    int expectedRange[5] = { 0, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 2);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestEraseRange)
{
    int range[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range, range + 3);

    deque.erase(deque.begin() + 1, deque.begin() + 3);

    int expectedRange[5] = { 0 };
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque(expectedRange, expectedRange + 1);
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestSwap)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);
    int range2[3] = { 3, 4, 5 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque2(range2, range2 + 3);

    swap(deque1, deque2);

    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque1(range2, range2 + 3);
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque2(range1, range1 + 3);
    EXPECT_EQ(expectedDeque1, deque1);
    EXPECT_EQ(expectedDeque2, deque2);
}

TEST(BoundedDequeTest, TestSwapWrapped)
{
    int range1[3] = { 0, 1, 2 };
    int range2[3] = { 3, 4, 5 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);
    infra::BoundedDeque<int>::WithMaxSize<5> deque2;
    deque2.push_front(range2[0]);
    deque2.push_back(range2[1]);
    deque2.push_back(range2[2]);

    swap(deque1, deque2);

    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque1(range2, range2 + 3);
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque2(range1, range1 + 3);
    EXPECT_EQ(expectedDeque1, deque1);
    EXPECT_EQ(expectedDeque2, deque2);
}

TEST(BoundedDequeTest, TestSwapDifferentSizes)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);
    int range2[2] = { 3, 4 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque2(range2, range2 + 2);

    swap(deque1, deque2);

    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque1(range2, range2 + 2);
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque2(range1, range1 + 3);
    EXPECT_EQ(expectedDeque1, deque1);
    EXPECT_EQ(expectedDeque2, deque2);
}

TEST(BoundedDequeTest, TestSwapDifferentSizesWrapped)
{
    int range1[2] = { 0, 1 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 2);
    int range2[3] = { 2, 3, 4 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque2;
    deque2.push_front(range2[1]);
    deque2.push_front(range2[0]);
    deque2.push_back(range2[2]);

    swap(deque1, deque2);

    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque1(range2, range2 + 3);
    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque2(range1, range1 + 2);
    EXPECT_EQ(expectedDeque1, deque1);
    EXPECT_EQ(expectedDeque2, deque2);
}

TEST(BoundedDequeTest, TestClear)
{
    infra::BoundedDeque<int>::WithMaxSize<5> deque(std::size_t(2), 4);
    deque.clear();

    infra::BoundedDeque<int>::WithMaxSize<5> expectedDeque;
    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestClearWithNonTrivialObject)
{
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> deque;
    deque.emplace_back(4);
    deque.clear();

    EXPECT_TRUE(deque.empty());
}

TEST(BoundedDequeTest, TestEmplace)
{
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> deque;
    deque.emplace_back(2);
    deque.emplace_back(3);
    deque.emplace(deque.begin() + 1, 4);

    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> expectedDeque;
    expectedDeque.emplace_back(2);
    expectedDeque.emplace_back(4);
    expectedDeque.emplace_back(3);

    EXPECT_EQ(expectedDeque, deque);
}

TEST(BoundedDequeTest, TestEmplaceWrapped)
{
    infra::BoundedDeque<infra::MoveConstructible>::WithMaxSize<5> deque;
    for (auto i = 0; i < 5; ++i)
        deque.emplace_back(1);
    deque.pop_front();
    deque.pop_front();
    deque.pop_front();
    deque.pop_front();

    deque.emplace_back(2);
    deque.emplace_back(3);
    deque.emplace(deque.begin() + 1, 4);

    EXPECT_EQ(infra::MoveConstructible(1), deque[0]);
    EXPECT_EQ(infra::MoveConstructible(4), deque[1]);
    EXPECT_EQ(infra::MoveConstructible(2), deque[2]);
    EXPECT_EQ(infra::MoveConstructible(3), deque[3]);
}

TEST(BoundedDequeTest, IteratorCopyConstruct)
{
    infra::BoundedDeque<int>::WithMaxSize<5> list(std::size_t(1), 4);

    infra::BoundedDeque<int>::iterator i(list.begin());
    EXPECT_EQ(4, *i);
}

TEST(BoundedDequeTest, IteratorArrow)
{
    infra::BoundedDeque<int>::WithMaxSize<5> list(std::size_t(1), 4);

    EXPECT_EQ(4, *(list.begin().operator->()));
}

TEST(BoundedDequeTest, IteratorRandomAccess)
{
    infra::BoundedDeque<int>::WithMaxSize<5> list(std::size_t(1), 4);

    infra::BoundedDeque<int>::iterator i(list.begin());
    EXPECT_EQ(4, i[0]);
}

TEST(BoundedDequeTest, IteratorPostInc)
{
    infra::BoundedDeque<int>::WithMaxSize<5> list(std::size_t(1), 4);

    infra::BoundedDeque<int>::iterator i(list.begin());
    EXPECT_EQ(4, *i++);
}

TEST(BoundedDequeTest, IteratorPostDec)
{
    infra::BoundedDeque<int>::WithMaxSize<5> list(std::size_t(2), 4);

    infra::BoundedDeque<int>::iterator i(std::next(list.begin()));
    EXPECT_EQ(4, *i--);
    EXPECT_EQ(list.begin(), i);
}

TEST(BoundedDequeTest, TestEquals)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);
    int range2[3] = { 3, 4, 5 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque2(range2, range2 + 3);
    int range3[2] = { 0, 1 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque3(range3, range3 + 2);

    EXPECT_TRUE(deque1 == deque1);
    EXPECT_FALSE(deque1 == deque2);
    EXPECT_FALSE(deque1 == deque3);
    EXPECT_FALSE(deque3 == deque1);
}

TEST(BoundedDequeTest, TestEqualsWrapped)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);
    infra::BoundedDeque<int>::WithMaxSize<5> deque2;
    deque2.push_front(range1[0]);
    deque2.push_back(range1[1]);
    deque2.push_back(range1[2]);

    EXPECT_TRUE(deque1 == deque2);
}

TEST(BoundedDequeTest, TestUnequals)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);

    EXPECT_FALSE(deque1 != deque1);
}

TEST(BoundedDequeTest, TestLessThan)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);
    int range2[3] = { 0, 4, 5 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque2(range2, range2 + 3);
    int range3[2] = { 0, 1 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque3(range3, range3 + 2);

    EXPECT_TRUE(deque1 < deque2);
    EXPECT_FALSE(deque1 < deque3);
    EXPECT_FALSE(deque1 < deque1);
}

TEST(BoundedDequeTest, TestLessThanWrapped)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);
    infra::BoundedDeque<int>::WithMaxSize<5> deque2;
    deque2.push_front(0);
    deque2.push_back(4);
    deque2.push_back(5);
    int range3[2] = { 0, 1 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque3(range3, range3 + 2);

    EXPECT_TRUE(deque1 < deque2);
    EXPECT_FALSE(deque1 < deque3);
    EXPECT_FALSE(deque1 < deque1);
}

TEST(BoundedDequeTest, TestGreaterThan)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);

    EXPECT_FALSE(deque1 > deque1);
}

TEST(BoundedDequeTest, TestLessThanOrEqual)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);

    EXPECT_TRUE(deque1 <= deque1);
}

TEST(BoundedDequeTest, TestGreaterThanOrEqual)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque1(range1, range1 + 3);

    EXPECT_TRUE(deque1 >= deque1);
}

TEST(BoundedDequeTest, IteratorComparison)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedDeque<int>::WithMaxSize<5> deque(range1, range1 + 3);

    EXPECT_TRUE(deque.begin() < deque.end());
    EXPECT_FALSE(deque.begin() > deque.end());
    EXPECT_TRUE(deque.begin() <= deque.end());
    EXPECT_FALSE(deque.begin() >= deque.end());
}
