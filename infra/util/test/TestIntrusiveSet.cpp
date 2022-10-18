#include "infra/util/IntrusiveSet.hpp"
#include "infra/util/ReverseRange.hpp"
#include "gtest/gtest.h"
#include <array>
#include <functional>
#include <memory>

namespace
{
    struct SetInt
        : infra::IntrusiveSet<SetInt>::NodeType
    {
        SetInt(int v)
            : value(v)
        {}

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
}

class IntrusiveSetTest
    : public testing::Test
{
public:
    std::string Representation(const infra::IntrusiveSet<SetInt>& tree)
    {
        EXPECT_TRUE(tree.invariant_holds());
        std::ostringstream os;

        if (!tree.empty())
            AddRepresentation(tree.root(), os);

        return os.str();
    }

    void AddRepresentation(const SetInt& node, std::ostream& os)
    {
        if (node.colour == SetInt::Colour::black)
            os << "b";
        else
            os << "r";

        os << node.value;

        if (node.left || node.right)
        {
            os << "(";
            if (node.left)
                AddRepresentation(*node.left, os);
            os << ",";
            if (node.right)
                AddRepresentation(*node.right, os);
            os << ")";
        }
    }
};

TEST_F(IntrusiveSetTest, TestConstructedEmpty)
{
    infra::IntrusiveSet<SetInt> set;

    EXPECT_TRUE(set.empty());
    EXPECT_EQ(0, set.size());
    EXPECT_TRUE(set.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestConstructionWithRange)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set(range, range + 3);

    EXPECT_EQ(3, set.size());

    EXPECT_EQ(SetInt(0), *set.begin());
    EXPECT_EQ(SetInt(1), *std::next(set.begin()));
    EXPECT_EQ(SetInt(2), *std::next(set.begin(), 2));
    EXPECT_TRUE(set.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestMoveConstruction)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> original(range, range + 3);
    infra::IntrusiveSet<SetInt> copy(std::move(original));

    EXPECT_EQ(3, copy.size());
    EXPECT_EQ(SetInt(0), copy.front());
    EXPECT_EQ(SetInt(2), copy.back());
    EXPECT_TRUE(original.empty());
    EXPECT_TRUE(copy.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestMoveAssignment)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> original(range, range + 3);
    infra::IntrusiveSet<SetInt> copy;
    copy = std::move(original);

    EXPECT_EQ(3, copy.size());
    EXPECT_EQ(SetInt(0), copy.front());
    EXPECT_EQ(SetInt(2), copy.back());
    EXPECT_TRUE(original.empty());
    EXPECT_TRUE(copy.invariant_holds());
}

TEST_F(IntrusiveSetTest, IterateAfterMoveAssignment)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> original(range, range + 3);
    infra::IntrusiveSet<SetInt> copy;
    copy = std::move(original);

    for (SetInt& i : copy)
    {}

    for (SetInt& i : MakeReverseRange(copy))
    {}
}

TEST_F(IntrusiveSetTest, TestBeginAndEnd)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set(range, range + 3);

    EXPECT_EQ(set.end(), std::next(set.begin(), 3));
    EXPECT_EQ(set.cbegin(), set.begin());
    EXPECT_EQ(set.cend(), set.end());

    EXPECT_EQ(SetInt(0), *set.begin());
}

TEST_F(IntrusiveSetTest, TestFront)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set(range, range + 3);

    EXPECT_EQ(SetInt(0), set.front());
}

TEST_F(IntrusiveSetTest, TestBack)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set(range, range + 3);

    EXPECT_EQ(SetInt(2), set.back());
}

TEST_F(IntrusiveSetTest, TestNotHasElement)
{
    infra::IntrusiveSet<SetInt> set;
    SetInt x(3);

    EXPECT_FALSE(set.has_element(x));
}

TEST_F(IntrusiveSetTest, TestHasElement)
{
    infra::IntrusiveSet<SetInt> set;
    SetInt x(3);
    set.insert(x);

    EXPECT_TRUE(set.has_element(x));
}

TEST_F(IntrusiveSetTest, TestNotHasElementForSameValue)
{
    infra::IntrusiveSet<SetInt> set;
    SetInt x(3);
    SetInt y(3);
    set.insert(x);

    EXPECT_FALSE(set.has_element(y));
}

TEST_F(IntrusiveSetTest, TestAssignRange)
{
    SetInt range[3] = { 0, 1, 2 };
    SetInt otherRange[2] = { 4, 5 };

    infra::IntrusiveSet<SetInt> set(range, range + 3);
    set.assign(otherRange, otherRange + 2);

    EXPECT_EQ(2, set.size());
    EXPECT_EQ(SetInt(4), set.front());
    EXPECT_EQ(SetInt(5), *std::next(set.begin()));
    EXPECT_TRUE(set.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestAssignN)
{
    SetInt range[3] = { 0, 1, 2 };
    SetInt range2[2] = { 3, 4 };

    infra::IntrusiveSet<SetInt> set(range, range + 3);
    set.assign(range2, range2 + 2);

    EXPECT_EQ(2, set.size());
    EXPECT_EQ(SetInt(3), set.front());
    EXPECT_EQ(SetInt(4), set.back());
    EXPECT_TRUE(set.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestSwap)
{
    SetInt range1[3] = { 0, 1, 2 };
    SetInt range2[3] = { 3, 4, 5 };
    infra::IntrusiveSet<SetInt> set1(range1, range1 + 3);
    infra::IntrusiveSet<SetInt> set2(range2, range2 + 3);

    swap(set1, set2);

    SetInt expectedRange1[3] = { 0, 1, 2 };
    SetInt expectedRange2[3] = { 3, 4, 5 };
    infra::IntrusiveSet<SetInt> expectedSet1(expectedRange2, expectedRange2 + 3);
    infra::IntrusiveSet<SetInt> expectedSet2(expectedRange1, expectedRange1 + 3);
    EXPECT_EQ(expectedSet1, set1);
    EXPECT_EQ(expectedSet2, set2);
    EXPECT_TRUE(set1.invariant_holds());
    EXPECT_TRUE(set2.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestSwapDifferentSizes)
{
    SetInt range1[3] = { 0, 1, 2 };
    SetInt range2[2] = { 3, 4 };
    infra::IntrusiveSet<SetInt> set1(range1, range1 + 3);
    infra::IntrusiveSet<SetInt> set2(range2, range2 + 2);

    swap(set1, set2);

    SetInt expectedRange1[3] = { 0, 1, 2 };
    SetInt expectedRange2[2] = { 3, 4 };
    infra::IntrusiveSet<SetInt> expectedSet1(expectedRange2, expectedRange2 + 2);
    infra::IntrusiveSet<SetInt> expectedSet2(expectedRange1, expectedRange1 + 3);
    EXPECT_EQ(expectedSet1, set1);
    EXPECT_EQ(expectedSet2, set2);
    EXPECT_TRUE(set1.invariant_holds());
    EXPECT_TRUE(set2.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestClear)
{
    SetInt range[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set(range, range + 3);
    set.clear();

    EXPECT_EQ(0, set.size());
    EXPECT_TRUE(set.empty());

    infra::IntrusiveSet<SetInt> expectedSet;
    EXPECT_EQ(expectedSet, set);
    EXPECT_TRUE(set.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestInsert)
{
    SetInt range[3] = { 0, 1, 2 };
    SetInt i1(3);
    infra::IntrusiveSet<SetInt> set(range, range + 3);
    set.insert(i1);

    EXPECT_EQ(4, set.size());
    EXPECT_EQ(i1, set.back());
    EXPECT_TRUE(set.invariant_holds());
}

TEST_F(IntrusiveSetTest, TestEraseSingleNode)
{
    std::array<SetInt, 1> range = { 1 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b1", Representation(set));

    set.erase(range[0]);
    EXPECT_EQ("", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackWithRedReplacement)
{
    std::array<SetInt, 2> range = { 2, 1 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b2(r1,)", Representation(set));

    set.erase(range[0]);
    EXPECT_EQ("b1", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackNonRootWithRedReplacement)
{
    SetInt range[4] = { 4, 3, 2, 1 };
    infra::IntrusiveSet<SetInt> set(range, range + 4);
    ASSERT_EQ("b3(b2(r1,),b4)", Representation(set));

    set.erase(range[2]);
    EXPECT_EQ("b3(b1,b4)", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseRedWithLeafReplacement)
{
    std::array<SetInt, 2> range = { 2, 1 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b2(r1,)", Representation(set));

    set.erase(range[1]);
    EXPECT_EQ("b2", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseRedWithBlackReplacement)
{
    std::array<SetInt, 6> range = { 3, 2, 1, 4, 5, 6 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    set.erase(range[5]);
    ASSERT_EQ("b2(b1,r4(b3,b5))", Representation(set));

    set.erase(range[3]);
    EXPECT_EQ("b2(b1,b5(r3,))", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackResultingIn32ai)
{
    std::array<SetInt, 5> range = { 4, 2, 5, 1, 3 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b4(b2(r1,r3),b5)", Representation(set));

    set.erase(range[2]);
    EXPECT_EQ("b2(b1,b4(r3,))", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackResultingIn32aiAndMoreRecolouring)
{
    std::array<SetInt, 7> range = { 5, 4, 6, 1, 0, 3, 2 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b5(r1(b0,b3(r2,r4)),b6)", Representation(set));

    set.erase(range[0]);
    EXPECT_EQ("b1(b0,r3(b2,b6(r4,)))", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackResultingIn32aii)
{
    std::array<SetInt, 4> range = { 3, 1, 4, 2 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b3(b1(,r2),b4)", Representation(set));

    set.erase(range[2]);
    EXPECT_EQ("b2(b1,b3)", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackResultingIn32aiii)
{
    std::array<SetInt, 4> range = { 3, 2, 4, 5 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b3(b2,b4(,r5))", Representation(set));

    set.erase(range[1]);
    EXPECT_EQ("b4(b3,b5)", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackResultingIn32aiv)
{
    std::array<SetInt, 5> range = { 3, 2, 5, 6, 4 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b3(b2,b5(r4,r6))", Representation(set));

    set.erase(range[1]);
    EXPECT_EQ("b4(b3,b5(,r6))", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackResultingIn32aivAndRecolouring)
{
    std::array<SetInt, 9> range = { 4, 0, 6, 3, 2, 5, 8, 1, 7 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    ASSERT_EQ("b4(r2(b0(,r1),b3),r6(b5,b8(r7,)))", Representation(set));

    set.erase(range[0]);
    EXPECT_EQ("b5(r2(b0(,r1),b3),r7(b6,b8))", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackResultingIn32ciAnd32b)
{
    std::array<SetInt, 6> range = { 4, 2, 5, 1, 3, 0 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    set.erase(range[5]);
    ASSERT_EQ("b4(r2(b1,b3),b5)", Representation(set));

    set.erase(range[2]);
    EXPECT_EQ("b2(b1,b4(r3,))", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseBlackResultingIn32ciiAnd32b)
{
    std::array<SetInt, 12> range = { 10, 2, 13, 1, 4, 12, 15, 3, 5, 11, 16, 6 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    set.erase(range[11]);
    ASSERT_EQ("b10(b2(b1,r4(b3,b5)),b13(b12(r11,),b15(,r16)))", Representation(set));

    set.erase(range[1]);
    EXPECT_EQ("b10(b3(b1,b4(,r5)),b13(b12(r11,),b15(,r16)))", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseRootWithBlackLeavesAndRedGrandLeave)
{
    std::array<SetInt, 5> range = { 0, 2, 1, 4, 3 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    set.erase(range[0]);
    ASSERT_EQ("b2(b1,b3(,r4))", Representation(set));

    set.erase(range[1]);
    EXPECT_EQ("b3(r1,r4)", Representation(set));
}

TEST_F(IntrusiveSetTest, TestEraseFollowedByRecolouringLeaf)
{
    std::array<SetInt, 12> range = { 5, 11, 6, 1, 9, 7, 4, 8, 0, 10, 3, 2 };
    infra::IntrusiveSet<SetInt> set(range.begin(), range.end());
    set.erase(range[0]);
    set.erase(range[1]);
    set.erase(range[2]);
    ASSERT_EQ("b7(b1(b0,r3(b2,b4)),b9(b8,b10))", Representation(set));

    set.erase(range[3]);
    EXPECT_EQ("b7(b2(b0,b3(,r4)),b9(b8,b10))", Representation(set));
}

TEST_F(IntrusiveSetTest, TestManyInsertionsAndDeletions)
{
    for (int size = 2; size != 40; ++size)
    {
        for (int step = 3; step != 7; ++step)
        {
            std::vector<SetInt> elements;
            int n = 0;
            std::generate_n(std::back_inserter(elements), size, [&n]()
                { return n++; });
            std::vector<std::size_t> indices;
            n = 0;
            std::generate_n(std::back_inserter(indices), size, [&n]()
                { return n++; });

            infra::IntrusiveSet<SetInt> set;
            int current = 0;
            while (!indices.empty())
            {
                current = (current + step) % indices.size();
                // std::cout << "insert size: " << size << ", step: " << step << ", set size: " << set.size() << ", insert: " << indices[current] << std::endl;
                set.insert(elements[indices[current]]);
                EXPECT_TRUE(set.invariant_holds());
                indices.erase(indices.begin() + current);
            }

            while (!set.empty())
            {
                current = (current + 2 * step) % set.size();
                // std::cout << "erase size: " << size << ", step: " << step << ", set size: " << set.size() << ", erase: " << std::next(set.begin(), current)->value << std::endl;
                set.erase(*std::next(set.begin(), current));
                EXPECT_TRUE(set.invariant_holds());
            }
        }
    }
}

TEST_F(IntrusiveSetTest, TestEquals)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set1(range1, range1 + 3);
    SetInt range2[3] = { 3, 4, 5 };
    infra::IntrusiveSet<SetInt> set2(range2, range2 + 3);
    SetInt range3[2] = { 0, 1 };
    infra::IntrusiveSet<SetInt> deque3(range3, range3 + 2);

    EXPECT_TRUE(set1 == set1);
    EXPECT_FALSE(set1 == set2);
    EXPECT_FALSE(set1 == deque3);
    EXPECT_FALSE(deque3 == set1);
}

TEST_F(IntrusiveSetTest, TestUnequals)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set1(range1, range1 + 3);

    EXPECT_FALSE(set1 != set1);
}

TEST_F(IntrusiveSetTest, TestLessThan)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set1(range1, range1 + 3);
    SetInt range2[3] = { 0, 4, 5 };
    infra::IntrusiveSet<SetInt> set2(range2, range2 + 3);
    SetInt range3[2] = { 0, 1 };
    infra::IntrusiveSet<SetInt> deque3(range3, range3 + 2);

    EXPECT_TRUE(set1 < set2);
    EXPECT_FALSE(set1 < deque3);
    EXPECT_FALSE(set1 < set1);
}

TEST_F(IntrusiveSetTest, TestGreaterThan)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set1(range1, range1 + 3);

    EXPECT_FALSE(set1 > set1);
}

TEST_F(IntrusiveSetTest, TestLessThanOrEqual)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set1(range1, range1 + 3);

    EXPECT_TRUE(set1 <= set1);
}

TEST_F(IntrusiveSetTest, TestGreaterThanOrEqual)
{
    SetInt range1[3] = { 0, 1, 2 };
    infra::IntrusiveSet<SetInt> set1(range1, range1 + 3);

    EXPECT_TRUE(set1 >= set1);
}
