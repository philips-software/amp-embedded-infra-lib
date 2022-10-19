#include "infra/util/IntrusiveForwardList.hpp"
#include "gtest/gtest.h"
#include <functional>
#include <memory>

struct ForwardListInt
    : infra::IntrusiveForwardList<ForwardListInt>::NodeType
{
    ForwardListInt(int v)
        : value(v)
    {}

    int value;

    bool operator==(const ForwardListInt& other) const
    {
        return value == other.value;
    }

    bool operator<(const ForwardListInt& other) const
    {
        return value < other.value;
    }
};

TEST(IntrusiveForwardListTest, TestConstructedEmpty)
{
    infra::IntrusiveForwardList<ForwardListInt> list;

    EXPECT_TRUE(list.empty());
}

TEST(IntrusiveForwardListTest, TestConstructionWithRange)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);

    EXPECT_EQ(ForwardListInt(0), *list.begin());
    EXPECT_EQ(ForwardListInt(1), *std::next(list.begin()));
    EXPECT_EQ(ForwardListInt(2), *std::next(list.begin(), 2));
}

TEST(IntrusiveForwardListTest, TestMoveConstruction)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> original(range, range + 3);
    infra::IntrusiveForwardList<ForwardListInt> copy(std::move(original));

    EXPECT_EQ(ForwardListInt(0), copy.front());
    EXPECT_EQ(ForwardListInt(1), *std::next(copy.begin()));
    EXPECT_EQ(ForwardListInt(2), *std::next(copy.begin(), 2));
    EXPECT_TRUE(original.empty());
}

TEST(IntrusiveForwardListTest, TestMoveAssignment)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> original(range, range + 3);
    infra::IntrusiveForwardList<ForwardListInt> copy;
    copy = std::move(original);

    EXPECT_EQ(ForwardListInt(0), copy.front());
    EXPECT_EQ(ForwardListInt(1), *std::next(copy.begin()));
    EXPECT_EQ(ForwardListInt(2), *std::next(copy.begin(), 2));
    EXPECT_TRUE(original.empty());
}

TEST(IntrusiveForwardListTest, IterateAfterMoveAssignment)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> original(range, range + 3);
    infra::IntrusiveForwardList<ForwardListInt> copy;
    copy = std::move(original);

    for (ForwardListInt& i : copy)
    {}
}

TEST(IntrusiveForwardListTest, TestBeginAndEnd)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);

    EXPECT_EQ(list.end(), std::next(list.begin(), 3));
    EXPECT_EQ(list.cbegin(), list.begin());
    EXPECT_EQ(list.cend(), list.end());

    EXPECT_EQ(ForwardListInt(0), *list.begin());
}

TEST(IntrusiveForwardListTest, TestFront)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);

    EXPECT_EQ(ForwardListInt(0), list.front());
}

TEST(IntrusiveForwardListTest, TestNotHasElement)
{
    infra::IntrusiveForwardList<ForwardListInt> list;
    ForwardListInt x(3);

    EXPECT_FALSE(list.has_element(x));
}

TEST(IntrusiveForwardListTest, TestHasElement)
{
    infra::IntrusiveForwardList<ForwardListInt> list;
    ForwardListInt x(3);
    list.push_front(x);

    EXPECT_TRUE(list.has_element(x));
}

TEST(IntrusiveForwardListTest, TestAssignRange)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);

    ForwardListInt otherRange[2] = { 4, 5 };
    list.assign(otherRange, otherRange + 2);

    EXPECT_EQ(ForwardListInt(4), list.front());
    EXPECT_EQ(ForwardListInt(5), *std::next(list.begin()));
}

TEST(IntrusiveForwardListTest, TestAssignN)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);

    ForwardListInt range2[2] = { 3, 4 };
    list.assign(range2, range2 + 2);

    EXPECT_EQ(ForwardListInt(3), list.front());
    EXPECT_EQ(ForwardListInt(4), *std::next(list.begin()));
}

TEST(IntrusiveForwardListTest, TestPushFront)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);
    ForwardListInt i1(1);
    list.push_front(i1);

    EXPECT_EQ(i1, list.front());
    EXPECT_EQ(ForwardListInt(0), *std::next(list.begin()));
}

TEST(IntrusiveForwardListTest, TestPopFront)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);
    list.pop_front();

    EXPECT_EQ(ForwardListInt(1), list.front());
}

TEST(IntrusiveForwardListTest, TestSwap)
{
    ForwardListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list1(range1, range1 + 3);
    ForwardListInt range2[3] = { 3, 4, 5 };
    infra::IntrusiveForwardList<ForwardListInt> list2(range2, range2 + 3);

    swap(list1, list2);

    infra::IntrusiveForwardList<ForwardListInt> expectedList1(range2, range2 + 3);
    infra::IntrusiveForwardList<ForwardListInt> expectedList2(range1, range1 + 3);
    EXPECT_EQ(expectedList1, list1);
    EXPECT_EQ(expectedList2, list2);
}

TEST(IntrusiveForwardListTest, TestSwapDifferentSizes)
{
    ForwardListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list1(range1, range1 + 3);
    ForwardListInt range2[2] = { 3, 4 };
    infra::IntrusiveForwardList<ForwardListInt> list2(range2, range2 + 2);

    swap(list1, list2);

    infra::IntrusiveForwardList<ForwardListInt> expectedList1(range2, range2 + 2);
    infra::IntrusiveForwardList<ForwardListInt> expectedList2(range1, range1 + 3);
    EXPECT_EQ(expectedList1, list1);
    EXPECT_EQ(expectedList2, list2);
}

TEST(IntrusiveForwardListTest, TestClear)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);
    list.clear();

    EXPECT_TRUE(list.empty());

    infra::IntrusiveForwardList<ForwardListInt> expectedList;
    EXPECT_EQ(expectedList, list);
}

TEST(IntrusiveForwardListTest, TestInsert)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);
    ForwardListInt i2(2);
    list.insert_after(list.begin(), i2);

    EXPECT_EQ(ForwardListInt(0), list.front());
    EXPECT_EQ(ForwardListInt(2), *std::next(list.begin()));
}

TEST(IntrusiveForwardListTest, TestEraseAfter)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);
    list.erase_after(range[0]);

    EXPECT_EQ(ForwardListInt(0), list.front());
    EXPECT_EQ(ForwardListInt(2), *std::next(list.begin()));
}

TEST(IntrusiveListTest, TestEraseSlowAtFront)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);
    list.erase_slow(range[0]);

    EXPECT_EQ(ForwardListInt(1), list.front());
    EXPECT_EQ(ForwardListInt(2), *std::next(list.begin()));
}

TEST(IntrusiveListTest, TestEraseSlow)
{
    ForwardListInt range[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list(range, range + 3);
    list.erase_slow(range[1]);

    EXPECT_EQ(ForwardListInt(0), list.front());
    EXPECT_EQ(ForwardListInt(2), *std::next(list.begin()));
}

TEST(IntrusiveForwardListTest, TestEquals)
{
    ForwardListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list1(range1, range1 + 3);
    ForwardListInt range2[3] = { 3, 4, 5 };
    infra::IntrusiveForwardList<ForwardListInt> list2(range2, range2 + 3);
    ForwardListInt range3[2] = { 0, 1 };
    infra::IntrusiveForwardList<ForwardListInt> deque3(range3, range3 + 2);

    EXPECT_TRUE(list1 == list1);
    EXPECT_FALSE(list1 == list2);
    EXPECT_FALSE(list1 == deque3);
    EXPECT_FALSE(deque3 == list1);
}

TEST(IntrusiveForwardListTest, TestUnequals)
{
    ForwardListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list1(range1, range1 + 3);

    EXPECT_FALSE(list1 != list1);
}

TEST(IntrusiveForwardListTest, TestLessThan)
{
    ForwardListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list1(range1, range1 + 3);
    ForwardListInt range2[3] = { 0, 4, 5 };
    infra::IntrusiveForwardList<ForwardListInt> list2(range2, range2 + 3);
    ForwardListInt range3[2] = { 0, 1 };
    infra::IntrusiveForwardList<ForwardListInt> deque3(range3, range3 + 2);

    EXPECT_TRUE(list1 < list2);
    EXPECT_FALSE(list1 < deque3);
    EXPECT_FALSE(list1 < list1);
}

TEST(IntrusiveForwardListTest, TestGreaterThan)
{
    ForwardListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list1(range1, range1 + 3);

    EXPECT_FALSE(list1 > list1);
}

TEST(IntrusiveForwardListTest, TestLessThanOrEqual)
{
    ForwardListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list1(range1, range1 + 3);

    EXPECT_TRUE(list1 <= list1);
}

TEST(IntrusiveForwardListTest, TestGreaterThanOrEqual)
{
    ForwardListInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveForwardList<ForwardListInt> list1(range1, range1 + 3);

    EXPECT_TRUE(list1 >= list1);
}
