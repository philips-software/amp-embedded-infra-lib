#include "upgrade/pack_builder/SparseVector.hpp"
#include "gtest/gtest.h"

class SparseVectorTest
    : public testing::Test
{
public:
    ~SparseVectorTest() override
    {
        EXPECT_TRUE(vector.InvariantHolds());
    }

    application::SparseVector<uint8_t> vector;
};

TEST_F(SparseVectorTest, ConstructedEmpty)
{
    EXPECT_TRUE(vector.Empty());
}

TEST_F(SparseVectorTest, InsertFirstValue)
{
    vector.Insert(12, 0);

    EXPECT_FALSE(vector.Empty());
    EXPECT_EQ(12, vector[0]);
}

TEST_F(SparseVectorTest, InsertSecondValueNonConsecutive)
{
    vector.Insert(12, 0);
    vector.Insert(14, 2);

    EXPECT_EQ(12, vector[0]);
    EXPECT_EQ(14, vector[2]);
}

TEST_F(SparseVectorTest, InsertSecondValueConsecutive)
{
    vector.Insert(12, 0);
    vector.Insert(14, 1);

    EXPECT_EQ(12, vector[0]);
    EXPECT_EQ(14, vector[1]);
}

TEST_F(SparseVectorTest, InsertValueTwiceThrowsException)
{
    vector.Insert(12, 0);

    EXPECT_THROW(vector.Insert(13, 0), application::OverwriteException);
}

TEST_F(SparseVectorTest, IterateFromBeginToEnd)
{
    vector.Insert(12, 0);
    vector.Insert(14, 1);
    vector.Insert(28, 4);

    std::vector<std::pair<std::size_t, uint8_t>> result;

    for (auto element : vector)
        result.push_back(element);

    EXPECT_EQ((std::vector<std::pair<std::size_t, uint8_t>>{ { 0, 12 }, { 1, 14 }, { 4, 28 } }), result);
}

TEST_F(SparseVectorTest, Equality)
{
    application::SparseVector<uint8_t> secondVector;

    EXPECT_EQ(vector, secondVector);

    secondVector.Insert(0, 0);
    EXPECT_NE(vector, secondVector);
}
