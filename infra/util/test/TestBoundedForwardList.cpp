#include "gtest/gtest.h"
#include "infra/util/BoundedForwardList.hpp"
#include "infra/util/test_helper/MoveConstructible.hpp"
#include <functional>
#include <memory>

TEST(BoundedForwardListTest, TestConstructedEmpty)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list;

    EXPECT_TRUE(list.empty());
    EXPECT_FALSE(list.full());
    EXPECT_EQ(0, list.size());
    EXPECT_EQ(5, list.max_size());
}

TEST(BoundedForwardListTest, TestConstructionWith2Elements)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(2), 4);

    EXPECT_FALSE(list.empty());
    EXPECT_FALSE(list.full());
    EXPECT_EQ(2, list.size());

    EXPECT_EQ(4, *list.begin());
    EXPECT_EQ(4, *std::next(list.begin()));
}

TEST(BoundedForwardListTest, TestConstructionWithRange)
{
    int range[3] = { 0, 1, 2};
    infra::BoundedForwardList<int>::WithMaxSize<5> list(range, range + 3);

    EXPECT_EQ(3, list.size());

    EXPECT_EQ(0, *list.begin());
    EXPECT_EQ(1, *std::next(list.begin()));
    EXPECT_EQ(2, *std::next(list.begin(), 2));
}

TEST(BoundedForwardListTest, TestConstructionWithInitializerList)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list({ 0, 1, 2 });

    EXPECT_EQ(3, list.size());

    EXPECT_EQ(0, *list.begin());
    EXPECT_EQ(1, *std::next(list.begin()));
    EXPECT_EQ(2, *std::next(list.begin(), 2));
}

TEST(BoundedForwardListTest, TestCopyConstruction)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> original(std::size_t(2), 4);
    infra::BoundedForwardList<int>::WithMaxSize<5> copy(original);

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(4, copy.front());
}

TEST(BoundedForwardListTest, TestMoveConstruction)
{
    infra::BoundedForwardList<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_front(2);
    infra::BoundedForwardList<infra::MoveConstructible>::WithMaxSize<5> copy(std::move(original));

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy.front().x);
}

TEST(BoundedForwardListTest, TestAssignment)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> original(std::size_t(2), 4);
    infra::BoundedForwardList<int>::WithMaxSize<5> copy;
    copy = original;

    EXPECT_EQ(2, copy.size());
    EXPECT_EQ(4, copy.front());
}

TEST(BoundedForwardListTest, TestSelfAssignment)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> original(std::size_t(2), 4);
    original = original;

    EXPECT_EQ(2, original.size());
    EXPECT_EQ(4, original.front());
}

TEST(BoundedForwardListTest, TestMove)
{
    infra::BoundedForwardList<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_front(2);
    infra::BoundedForwardList<infra::MoveConstructible>::WithMaxSize<5> copy;
    copy = std::move(original);

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy.front().x);
}

TEST(BoundedForwardListTest, TestBeginAndEnd)
{
    int range[3] = { 0, 1, 2};
    infra::BoundedForwardList<int>::WithMaxSize<5> list(range, range + 3);

    EXPECT_EQ(list.end(), std::next(list.begin(), 3));
    EXPECT_EQ(list.cbegin(), list.begin());
    EXPECT_EQ(list.cend(), list.end());

    EXPECT_EQ(0, *list.begin());
}

TEST(BoundedForwardListTest, TestFull)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(5), 4);

    EXPECT_TRUE(list.full());
}

TEST(BoundedForwardListTest, TestFront)
{
    int range[3] = { 0, 1, 2};
    infra::BoundedForwardList<int>::WithMaxSize<5> list(range, range + 3);

    EXPECT_EQ(0, list.front());
}

TEST(BoundedForwardListTest, TestAssignRange)
{
    int range[3] = { 0, 1, 2};
    infra::BoundedForwardList<int>::WithMaxSize<5> list(range, range + 3);

    int otherRange[2] = { 4, 5 };
    list.assign(otherRange, otherRange + 2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(4, list.front());
    EXPECT_EQ(5, *std::next(list.begin()));
}

TEST(BoundedForwardListTest, TestAssignN)
{
    int range[3] = { 0, 1, 2};
    infra::BoundedForwardList<int>::WithMaxSize<5> list(range, range + 3);

    list.assign(std::size_t(2), 4);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(4, list.front());
    EXPECT_EQ(4, *std::next(list.begin()));
}

TEST(BoundedForwardListTest, TestMoveFromRange)
{
    infra::BoundedForwardList<infra::MoveConstructible>::WithMaxSize<5> original;
    original.emplace_front(2);
    infra::BoundedForwardList<infra::MoveConstructible>::WithMaxSize<5> copy;
    copy.move_from_range(original.begin(), original.end());

    EXPECT_EQ(1, copy.size());
    EXPECT_EQ(2, copy.front().x);
}

TEST(BoundedForwardListTest, TestPushFront)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(2), 4);
    list.push_front(1);

    EXPECT_EQ(3, list.size());
    EXPECT_EQ(1, list.front());
}

TEST(BoundedForwardListTest, TestPopFront)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(2), 4);
    list.pop_front();

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(4, list.front());
}

TEST(BoundedForwardListTest, TestSwap)
{
    int range1[3] = { 0, 1, 2};
    infra::BoundedForwardList<int>::WithMaxSize<5> list1(range1, range1 + 3);
    int range2[3] = { 3, 4, 5};
    infra::BoundedForwardList<int>::WithMaxSize<5> list2(range2, range2 + 3);

    swap(list1, list2);

    infra::BoundedForwardList<int>::WithMaxSize<5> expectedList1(range2, range2 + 3);
    infra::BoundedForwardList<int>::WithMaxSize<5> expectedList2(range1, range1 + 3);
    EXPECT_EQ(expectedList1, list1);
    EXPECT_EQ(expectedList2, list2);
}

TEST(BoundedForwardListTest, TestSwapDifferentSizes)
{
    int range1[3] = { 0, 1, 2};
    infra::BoundedForwardList<int>::WithMaxSize<5> list1(range1, range1 + 3);
    int range2[2] = { 3, 4};
    infra::BoundedForwardList<int>::WithMaxSize<5> list2(range2, range2 + 2);

    swap(list1, list2);

    infra::BoundedForwardList<int>::WithMaxSize<5> expectedList1(range2, range2 + 2);
    infra::BoundedForwardList<int>::WithMaxSize<5> expectedList2(range1, range1 + 3);
    EXPECT_EQ(expectedList1, list1);
    EXPECT_EQ(expectedList2, list2);
}

TEST(BoundedForwardListTest, TestClear)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(2), 4);
    list.clear();

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.empty());

    infra::BoundedForwardList<int>::WithMaxSize<5> expectedList;
    EXPECT_EQ(expectedList, list);
}

TEST(BoundedForwardListTest, TestInsertAfter)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(1), 4);
    list.insert_after(list.begin(), 2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, *std::next(list.begin()));
}

TEST(BoundedForwardList, TestErase)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(3), 4);

    list.erase_slow(std::next(list.begin(), 2));
    EXPECT_EQ(2, list.size());
    EXPECT_EQ(2, std::distance(list.begin(), list.end()));

    list.erase_slow(list.begin());
    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, std::distance(list.begin(), list.end()));
}

TEST(BoundedForwardListTest, TestEraseAllAfter)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(3), 4);

    list.erase_all_after(list.begin());
    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, std::distance(list.begin(), list.end()));
}

TEST(BoundedForwardListTest, TestEraseAllAfterNothing)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(1), 4);

    list.erase_all_after(list.begin());
    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, std::distance(list.begin(), list.end()));
}

TEST(BoundedForwardListTest, CopyConstructIterator)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(1), 4);

    infra::BoundedForwardList<int>::iterator i(list.begin());
    EXPECT_EQ(4, *i);
}

TEST(BoundedForwardListTest, Arrow)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(1), 4);

    EXPECT_EQ(4, *(list.begin().operator->()));
}

TEST(BoundedForwardListTest, PostInc)
{
    infra::BoundedForwardList<int>::WithMaxSize<5> list(std::size_t(1), 4);

    infra::BoundedForwardList<int>::iterator i(list.begin());
    EXPECT_EQ(4, *i++);
}

TEST(BoundedForwardListTest, TestEquals)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedForwardList<int>::WithMaxSize<5> list1(range1, range1 + 3);
    int range2[3] = { 3, 4, 5 };
    infra::BoundedForwardList<int>::WithMaxSize<5> list2(range2, range2 + 3);
    int range3[2] = { 0, 1 };
    infra::BoundedForwardList<int>::WithMaxSize<5> deque3(range3, range3 + 2);

    EXPECT_TRUE(list1 == list1);
    EXPECT_FALSE(list1 == list2);
    EXPECT_FALSE(list1 == deque3);
    EXPECT_FALSE(deque3 == list1);
}

TEST(BoundedForwardListTest, TestUnequals)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedForwardList<int>::WithMaxSize<5> list1(range1, range1 + 3);

    EXPECT_FALSE(list1 != list1);
}

TEST(BoundedForwardListTest, TestLessThan)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedForwardList<int>::WithMaxSize<5> list1(range1, range1 + 3);
    int range2[3] = { 0, 4, 5 };
    infra::BoundedForwardList<int>::WithMaxSize<5> list2(range2, range2 + 3);
    int range3[2] = { 0, 1 };
    infra::BoundedForwardList<int>::WithMaxSize<5> deque3(range3, range3 + 2);

    EXPECT_TRUE(list1 < list2);
    EXPECT_FALSE(list1 < deque3);
    EXPECT_FALSE(list1 < list1);
}

TEST(BoundedForwardListTest, TestGreaterThan)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedForwardList<int>::WithMaxSize<5> list1(range1, range1 + 3);

    EXPECT_FALSE(list1 > list1);
}

TEST(BoundedForwardListTest, TestLessThanOrEqual)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedForwardList<int>::WithMaxSize<5> list1(range1, range1 + 3);

    EXPECT_TRUE(list1 <= list1);
}

TEST(BoundedForwardListTest, TestGreaterThanOrEqual)
{
    int range1[3] = { 0, 1, 2 };
    infra::BoundedForwardList<int>::WithMaxSize<5> list1(range1, range1 + 3);

    EXPECT_TRUE(list1 >= list1);
}
