#include "gtest/gtest.h"
#include "infra/util/IntrusiveList.hpp"
#include "infra/util/ReverseRange.hpp"
#include <functional>
#include <memory>
#include <utility>

struct ListInt
    : infra::IntrusiveList<ListInt>::NodeType
{
    ListInt(int v): value(v) {}

    int value;

    bool operator==(const ListInt& other) const
    {
        return value == other.value;
    }

    bool operator<(const ListInt& other) const
    {
        return value < other.value;
    }
};

TEST(IntrusiveListTest, TestConstructedEmpty)
{
    infra::IntrusiveList<ListInt> list;

    EXPECT_TRUE(list.empty());
    EXPECT_EQ(0, list.size());
}

TEST(IntrusiveListTest, TestConstructionWithRange)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);

    EXPECT_EQ(3, list.size());

    EXPECT_EQ(ListInt(0), *list.begin());
    EXPECT_EQ(ListInt(1), *std::next(list.begin()));
    EXPECT_EQ(ListInt(2), *std::next(list.begin(), 2));
}

TEST(IntrusiveListTest, TestMoveConstruction)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> original(range, range + 3);
    infra::IntrusiveList<ListInt> copy(std::move(original));

    EXPECT_EQ(3, copy.size());
    EXPECT_EQ(ListInt(0), copy.front());
    EXPECT_EQ(ListInt(2), copy.back());
    EXPECT_TRUE(original.empty());
}

TEST(IntrusiveListTest, TestMoveAssignment)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> original(range, range + 3);
    infra::IntrusiveList<ListInt> copy;
    copy = std::move(original);

    EXPECT_EQ(3, copy.size());
    EXPECT_EQ(ListInt(0), copy.front());
    EXPECT_EQ(ListInt(2), copy.back());
    EXPECT_TRUE(original.empty());
}

TEST(IntrusiveListTest, TestMoveAssignmentFromEmptyList)
{
    infra::IntrusiveList<ListInt> original;
    infra::IntrusiveList<ListInt> copy;
    copy = std::move(original);

    EXPECT_EQ(0, copy.size());
}

TEST(IntrusiveListTest, IterateAfterMoveAssignment)
{
    ListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveList<ListInt> original(range, range + 3);
    infra::IntrusiveList<ListInt> copy;
    copy = std::move(original);

    for (ListInt& i: copy)
    {}

    for (ListInt& i: MakeReverseRange(copy))
    {}
}

TEST(IntrusiveListTest, TestBeginAndEnd)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);

    EXPECT_EQ(list.end(), std::next(list.begin(), 3));
    EXPECT_EQ(list.cbegin(), list.begin());
    EXPECT_EQ(list.cend(), list.end());

    EXPECT_EQ(ListInt(0), *list.begin());
}

TEST(IntrusiveListTest, TestFront)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);

    EXPECT_EQ(ListInt(0), list.front());
}

TEST(IntrusiveListTest, TestBack)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);

    EXPECT_EQ(ListInt(2), list.back());
}

TEST(IntrusiveListTest, TestNotHasElement)
{
    infra::IntrusiveList<ListInt> list;
    ListInt x(3);

    EXPECT_FALSE(list.has_element(x));
}

TEST(IntrusiveListTest, TestHasElement)
{
    infra::IntrusiveList<ListInt> list;
    ListInt x(3);
    list.push_back(x);

    EXPECT_TRUE(list.has_element(x));
}

TEST(IntrusiveListTest, TestAssignRange)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);

    ListInt otherRange[2] = { 4, 5 };
    list.assign(otherRange, otherRange + 2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(ListInt(4), list.front());
    EXPECT_EQ(ListInt(5), *std::next(list.begin()));
}

TEST(IntrusiveListTest, TestAssignN)
{
    ListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveList<ListInt> list(range, range + 3);

    ListInt range2[2] = { 3, 4 };
    list.assign(range2, range2 + 2);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(ListInt(3), list.front());
    EXPECT_EQ(ListInt(4), list.back());
}

TEST(IntrusiveListTest, TestPushFront)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);
    ListInt i1(1);
    list.push_front(i1);

    EXPECT_EQ(4, list.size());
    EXPECT_EQ(i1, list.front());
}

TEST(IntrusiveListTest, TestPopFront)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);
    list.pop_front();

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(ListInt(1), list.front());
}

TEST(IntrusiveListTest, TestPushBack)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);
    ListInt i1(1);
    list.push_back(i1);

    EXPECT_EQ(4, list.size());
    EXPECT_EQ(i1, list.back());
}

TEST(IntrusiveListTest, TestPopBack)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);
    list.pop_back();

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(ListInt(1), list.back());
}

TEST(IntrusiveListTest, TestSwap)
{
    ListInt range1[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list1(range1, range1 + 3);
    ListInt range2[3] = { 3, 4, 5};
    infra::IntrusiveList<ListInt> list2(range2, range2 + 3);

    swap(list1, list2);

    ListInt range1Copy[3] = { 0, 1, 2};
    ListInt range2Copy[3] = { 3, 4, 5};
    infra::IntrusiveList<ListInt> expectedList1(range2Copy, range2Copy + 3);
    infra::IntrusiveList<ListInt> expectedList2(range1Copy, range1Copy + 3);
    EXPECT_EQ(expectedList1, list1);
    EXPECT_EQ(expectedList2, list2);
}

TEST(IntrusiveListTest, TestSwapDifferentSizes)
{
    ListInt range1[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list1(range1, range1 + 3);
    ListInt range2[2] = { 3, 4};
    infra::IntrusiveList<ListInt> list2(range2, range2 + 2);

    swap(list1, list2);

    ListInt range1Copy[3] = { 0, 1, 2};
    ListInt range2Copy[2] = { 3, 4};
    infra::IntrusiveList<ListInt> expectedList1(range2Copy, range2Copy + 2);
    infra::IntrusiveList<ListInt> expectedList2(range1Copy, range1Copy + 3);
    EXPECT_EQ(expectedList1, list1);
    EXPECT_EQ(expectedList2, list2);
}

TEST(IntrusiveListTest, TestClear)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);
    list.clear();

    EXPECT_EQ(0, list.size());
    EXPECT_TRUE(list.empty());

    infra::IntrusiveList<ListInt> expectedList;
    EXPECT_EQ(expectedList, list);
}

TEST(IntrusiveListTest, TestInsert)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);
    ListInt i2(2);
    list.insert(std::next(list.begin()), i2);

    EXPECT_EQ(4, list.size());
    EXPECT_EQ(ListInt(2), *std::next(list.begin()));
}

TEST(IntrusiveListTest, TestErase)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);
    list.erase(range[1]);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(ListInt(2), *std::next(list.begin()));
}

TEST(IntrusiveListTest, TestDoubleErase)
{
    ListInt range[3] = { 0, 1, 2};
    infra::IntrusiveList<ListInt> list(range, range + 3);
    list.erase(range[1]);
    list.erase(range[1]);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(ListInt(2), *std::next(list.begin()));
}

TEST(IntrusiveListTest, TestEquals)
{
    ListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveList<ListInt> list1(range1, range1 + 3);
    ListInt range2[3] = { 3, 4, 5 };
    infra::IntrusiveList<ListInt> list2(range2, range2 + 3);
    ListInt range3[2] = { 0, 1 };
    infra::IntrusiveList<ListInt> deque3(range3, range3 + 2);

    EXPECT_TRUE(list1 == list1);
    EXPECT_FALSE(list1 == list2);
    EXPECT_FALSE(list1 == deque3);
    EXPECT_FALSE(deque3 == list1);
}

TEST(IntrusiveListTest, TestUnequals)
{
    ListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveList<ListInt> list1(range1, range1 + 3);

    EXPECT_FALSE(list1 != list1);
}

TEST(IntrusiveListTest, TestLessThan)
{
    ListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveList<ListInt> list1(range1, range1 + 3);
    ListInt range2[3] = { 0, 4, 5 };
    infra::IntrusiveList<ListInt> list2(range2, range2 + 3);
    ListInt range3[2] = { 0, 1 };
    infra::IntrusiveList<ListInt> deque3(range3, range3 + 2);

    EXPECT_TRUE(list1 < list2);
    EXPECT_FALSE(list1 < deque3);
    EXPECT_FALSE(list1 < list1);
}

TEST(IntrusiveListTest, TestGreaterThan)
{
    ListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveList<ListInt> list1(range1, range1 + 3);

    EXPECT_FALSE(list1 > list1);
}

TEST(IntrusiveListTest, TestLessThanOrEqual)
{
    ListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveList<ListInt> list1(range1, range1 + 3);

    EXPECT_TRUE(list1 <= list1);
}

TEST(IntrusiveListTest, TestGreaterThanOrEqual)
{
    ListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveList<ListInt> list1(range1, range1 + 3);

    EXPECT_TRUE(list1 >= list1);
}
