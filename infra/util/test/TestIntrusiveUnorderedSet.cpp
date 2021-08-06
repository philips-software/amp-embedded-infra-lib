#include "gtest/gtest.h"
#include "infra/util/IntrusiveUnorderedSet.hpp"
#include "infra/util/ReverseRange.hpp"

struct SetInt
    : infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4>::NodeType
{
    SetInt(int v): value(v) {}

    int value;

    bool operator==(const SetInt& other) const
    {
        return value == other.value;
    }

    bool operator<(const SetInt& other) const
    {
        return value < other.value;
    }
};

template<>
struct std::hash<SetInt>
{
    std::size_t operator()(SetInt i) const
    {
        return std::hash<int>()(i.value);
    }
};

bool SetEquals(infra::IntrusiveUnorderedSet<SetInt>& set, infra::MemoryRange<SetInt> range)
{
    return std::set<SetInt>(set.begin(), set.end()) == std::set<SetInt>(range.begin(), range.end());
}

TEST(IntrusiveUnorderedSetTest, TestConstructedEmpty)
{
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set;

    EXPECT_TRUE(set.empty());
    EXPECT_EQ(0, set.size());
}

TEST(IntrusiveUnorderedSetTest, TestConstructionWithRange)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);

    EXPECT_TRUE(SetEquals(set, infra::MakeRange(range)));
}

TEST(IntrusiveUnorderedSetTest, TestMoveConstruction)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> original(range, range + 3);
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> copy(std::move(original));

    EXPECT_TRUE(original.empty());
    EXPECT_TRUE(SetEquals(copy, infra::MakeRange(range)));
}

TEST(IntrusiveUnorderedSetTest, TestMoveAssignment)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> original(range, range + 3);
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> copy;
    copy = std::move(original);

    EXPECT_TRUE(original.empty());
    EXPECT_TRUE(SetEquals(copy, infra::MakeRange(range)));
}

TEST(IntrusiveUnorderedSetTest, TestMoveAssignmentFromEmptyList)
{
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> original;
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> copy;
    copy = std::move(original);

    EXPECT_EQ(0, copy.size());
}

TEST(IntrusiveUnorderedSetTest, IterateAfterMoveAssignment)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> original(range, range + 3);
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> copy;
    copy = std::move(original);

    for (SetInt& i: copy)
    {}

    for (SetInt& i : infra::MakeReverseRange(copy))
    {}
}

TEST(IntrusiveUnorderedSetTest, TestBeginAndEnd)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);

    EXPECT_EQ(set.end(), std::next(set.begin(), 3));
    EXPECT_EQ(set.cbegin(), set.begin());
    EXPECT_EQ(set.cend(), set.end());

    EXPECT_EQ(SetInt(0), *set.begin());

    const auto& set2 = set;

    EXPECT_EQ(set2.end(), std::next(set2.begin(), 3));
    EXPECT_EQ(set2.cbegin(), set2.begin());
    EXPECT_EQ(set2.cend(), set2.end());
}


TEST(IntrusiveUnorderedSetTest, TestRBeginAndREnd)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);

    EXPECT_EQ(set.rend(), std::next(set.rbegin(), 3));
    EXPECT_EQ(set.crbegin(), set.rbegin());
    EXPECT_EQ(set.crend(), set.rend());

    EXPECT_EQ(SetInt(0), *std::prev(set.rend()));

    const auto& set2 = set;

    EXPECT_EQ(set2.rend(), std::next(set2.rbegin(), 3));
    EXPECT_EQ(set2.crbegin(), set2.rbegin());
    EXPECT_EQ(set2.crend(), set2.rend());
}

TEST(IntrusiveUnorderedSetTest, TestHasElement)
{
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set;
    SetInt x(3);
    set.insert(x);
    SetInt y(4);

    EXPECT_TRUE(set.has_element(x));
    EXPECT_FALSE(set.has_element(y));
}

TEST(IntrusiveUnorderedSetTest, TestContains)
{
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set;
    SetInt x(3);
    set.insert(x);
    SetInt y(4);

    SetInt x2(3);
    SetInt y2(4);

    EXPECT_TRUE(set.contains(x2));
    EXPECT_FALSE(set.contains(y2));
}

TEST(IntrusiveUnorderedSetTest, TestAssignRange)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);

    SetInt otherRange[2] = { 4, 5 };
    set.assign(otherRange, otherRange + 2);

    EXPECT_TRUE(SetEquals(set, infra::MakeRange(otherRange)));
}

TEST(IntrusiveUnorderedSetTest, TestAssignN)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);

    SetInt otherRange[2] = { 3, 4 };
    set.assign(otherRange, otherRange + 2);

    EXPECT_TRUE(SetEquals(set, infra::MakeRange(otherRange)));
}

TEST(IntrusiveUnorderedSetTest, TestSwap)
{
    SetInt range1[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set1(range1, range1 + 3);
    SetInt range2[3] = { 3, 4, 5};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set2(range2, range2 + 3);

    swap(set1, set2);

    SetInt range1Copy[3] = { 0, 1, 2};
    SetInt range2Copy[3] = { 3, 4, 5};
    EXPECT_TRUE(SetEquals(set1, infra::MakeRange(range2Copy)));
    EXPECT_TRUE(SetEquals(set2, infra::MakeRange(range1Copy)));
}

TEST(IntrusiveUnorderedSetTest, TestSwapDifferentSizes)
{
    SetInt range1[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set1(range1, range1 + 3);
    SetInt range2[2] = { 3, 4};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set2(range2, range2 + 2);

    swap(set1, set2);

    SetInt range1Copy[3] = { 0, 1, 2};
    SetInt range2Copy[2] = { 3, 4};
    EXPECT_TRUE(SetEquals(set1, infra::MakeRange(range2Copy)));
    EXPECT_TRUE(SetEquals(set2, infra::MakeRange(range1Copy)));
}

TEST(IntrusiveUnorderedSetTest, TestSwapDifferentBucketSizes)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set1(range1, range1 + 3);
    SetInt range2[3] = { 3, 4, 5 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<2> set2(range2, range2 + 3);

    swap(set1, set2);

    SetInt range1Copy[3] = { 0, 1, 2 };
    SetInt range2Copy[3] = { 3, 4, 5 };
    EXPECT_TRUE(SetEquals(set1, infra::MakeRange(range2Copy)));
    EXPECT_TRUE(SetEquals(set2, infra::MakeRange(range1Copy)));
}

TEST(IntrusiveUnorderedSetTest, TestClear)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);
    set.clear();

    EXPECT_EQ(0, set.size());
    EXPECT_TRUE(set.empty());
}

TEST(IntrusiveUnorderedSetTest, TestInsert)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);

    SetInt i2(2);
    set.insert(i2);

    EXPECT_TRUE(SetEquals(set, infra::MakeRange(range)));
}

TEST(IntrusiveUnorderedSetTest, TestErase)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);
    set.erase(range[1]);

    SetInt newRange[2] = { 0, 2 };
    EXPECT_TRUE(SetEquals(set, infra::MakeRange(newRange)));
}

TEST(IntrusiveUnorderedSetTest, TestManyInsertErase)
{
    SetInt range[25] = { 0, 13, 18, 1, 19, 2, 14, 3, 21, 4, 5, 25, 6, 33, 7, 17, 16, 8, 15, 9, 30, 10, 29, 11, 12 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 25);

    for (auto& v: range)
        set.erase(v);

    EXPECT_TRUE(set.empty());
}

TEST(IntrusiveUnorderedSetTest, TestDoubleErase)
{
    SetInt range[3] = { 0, 1, 2};
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set(range, range + 3);
    set.erase(range[1]);
    set.erase(range[1]);

    SetInt newRange[2] = { 0, 2 };
    EXPECT_TRUE(SetEquals(set, infra::MakeRange(newRange)));
}

TEST(IntrusiveUnorderedSetTest, TestEquals)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set1(range1, range1 + 3);
    SetInt range2[3] = { 3, 4, 5 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set2(range2, range2 + 3);
    SetInt range3[2] = { 0, 1 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set3(range3, range3 + 2);

    EXPECT_TRUE(set1 == set1);
    EXPECT_FALSE(set1 == set2);
    EXPECT_FALSE(set1 == set3);
    EXPECT_FALSE(set3 == set1);
}

TEST(IntrusiveUnorderedSetTest, TestUnequals)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveUnorderedSet<SetInt>::WithBuckets<4> set1(range1, range1 + 3);

    EXPECT_FALSE(set1 != set1);
}
