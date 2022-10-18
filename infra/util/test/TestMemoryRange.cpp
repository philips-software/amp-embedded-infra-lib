#include "infra/util/BoundedVector.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/MemoryRange.hpp"
#include "gtest/gtest.h"

TEST(MemoryRangeTest, TestConstructedEmpty)
{
    infra::MemoryRange<int> range;

    EXPECT_TRUE(range.empty());
    EXPECT_EQ(0, range.size());
}

TEST(MemoryRangeTest, TestConstructionWithPointers)
{
    int array[] = { 0, 1, 2 };
    infra::MemoryRange<int> range(array, array + 3);

    EXPECT_FALSE(range.empty());
    EXPECT_EQ(3, range.size());
    EXPECT_EQ(&array[0], range.begin());
}

TEST(MemoryRangeTest, TestCopyConstruction)
{
    int array[] = { 0, 1, 2 };
    infra::MemoryRange<int> range(array, array + 3);
    infra::MemoryRange<const int> copy(range);

    EXPECT_FALSE(copy.empty());
    EXPECT_EQ(3, copy.size());
    EXPECT_EQ(&array[0], copy.begin());
}

TEST(MemoryRangeTest, TestConstructionWithArray)
{
    std::array<int, 3> array = { 0, 1, 2 };
    infra::MemoryRange<int> range(array);

    EXPECT_FALSE(range.empty());
    EXPECT_EQ(3, range.size());
    EXPECT_EQ(&array[0], range.begin());
}

TEST(MemoryRangeTest, TestConstructionWithEmptyArray)
{
    std::array<int, 0> array;
    infra::MemoryRange<int> range(array);

    EXPECT_TRUE(range.empty());
    EXPECT_EQ(0, range.size());
}

TEST(MemoryRangeTest, TestConstructionWithVector)
{
    std::vector<int> vector = { 0, 1, 2 };
    infra::MemoryRange<int> range(vector);

    EXPECT_FALSE(range.empty());
    EXPECT_EQ(3, range.size());
    EXPECT_EQ(&vector[0], range.begin());
}

TEST(MemoryRangeTest, TestEnd)
{
    std::array<int, 3> array = { 0, 1, 2 };
    infra::MemoryRange<int> range(array);

    EXPECT_EQ(range.begin() + 3, range.end());
    EXPECT_EQ(range.cbegin() + 3, range.cend());
}

TEST(MemoryRangeTest, TestIndex)
{
    std::array<int, 3> array = { 0, 1, 2 };
    infra::MemoryRange<int> range(array);

    EXPECT_EQ(0, range[0]);
    EXPECT_EQ(1, range[1]);
    EXPECT_EQ(2, range[2]);
}

TEST(MemoryRangeTest, TestFront)
{
    std::array<int, 3> array = { 0, 1, 2 };
    infra::MemoryRange<int> range(array);

    EXPECT_EQ(0, range.front());
}

TEST(MemoryRangeTest, TestBack)
{
    std::array<int, 3> array = { 0, 1, 2 };
    infra::MemoryRange<int> range(array);

    EXPECT_EQ(2, range.back());
}

TEST(MemoryRangeTest, TestContains)
{
    std::array<int, 5> array = { 0, 1, 2, 3, 4 };
    infra::MemoryRange<int> range(array.data() + 1, array.data() + 3);

    EXPECT_FALSE(range.contains(array.data()));
    EXPECT_TRUE(range.contains(array.data() + 1));
    EXPECT_TRUE(range.contains(array.data() + 2));
    EXPECT_FALSE(range.contains(array.data() + 3));
}

TEST(MemoryRangeTest, TestContainsOrEnd)
{
    std::array<int, 5> array = { 0, 1, 2, 3, 4 };
    infra::MemoryRange<int> range(array.data() + 1, array.data() + 3);

    EXPECT_FALSE(range.contains_or_end(array.data()));
    EXPECT_TRUE(range.contains_or_end(array.data() + 1));
    EXPECT_TRUE(range.contains_or_end(array.data() + 2));
    EXPECT_TRUE(range.contains_or_end(array.data() + 3));
    EXPECT_FALSE(range.contains_or_end(array.data() + 4));
}

TEST(MemoryRangeTest, TestClear)
{
    std::array<int, 3> array = { 0, 1, 2 };
    infra::MemoryRange<int> range(array);
    range.clear();

    EXPECT_TRUE(range.empty());
}

TEST(MemoryRangeTest, TestPopFront)
{
    std::array<int, 4> array = { 0, 1, 2, 3 };
    infra::MemoryRange<int> range(array);

    range.pop_front();
    EXPECT_EQ(1, range.front());
}

TEST(MemoryRangeTest, TestPopBack)
{
    std::array<int, 4> array = { 0, 1, 2, 3 };
    infra::MemoryRange<int> range(array);

    range.pop_back();
    EXPECT_EQ(2, range.back());
}

TEST(MemoryRangeTest, TestShrinkFromFront)
{
    std::array<int, 4> array = { 0, 1, 2, 3 };
    infra::MemoryRange<int> range(array);

    range.shrink_from_front_to(3);
    EXPECT_EQ((std::array<int, 3>{ 1, 2, 3 }), range);
}

TEST(MemoryRangeTest, TestShrinkFromFrontDoesNotShrink)
{
    std::array<int, 4> array = { 0, 1, 2, 3 };
    infra::MemoryRange<int> range(array);

    range.shrink_from_front_to(6);
    EXPECT_EQ((std::array<int, 4>{ 0, 1, 2, 3 }), range);
}

TEST(MemoryRangeTest, TestShrinkFromBack)
{
    std::array<int, 4> array = { 0, 1, 2, 3 };
    infra::MemoryRange<int> range(array);

    range.shrink_from_back_to(3);
    EXPECT_EQ((std::array<int, 3>{ 0, 1, 2 }), range);
}

TEST(MemoryRangeTest, TestShrinkFromBackDoesNotShrink)
{
    std::array<int, 4> array = { 0, 1, 2, 3 };
    infra::MemoryRange<int> range(array);

    range.shrink_from_back_to(6);
    EXPECT_EQ((std::array<int, 4>{ 0, 1, 2, 3 }), range);
}

TEST(MemoryRangeTest, Copy)
{
    int x = 0x12345678;
    int y;

    infra::Copy(infra::MakeByteRange(x), infra::MakeByteRange(y));
    EXPECT_EQ(0x12345678, y);
}

TEST(MemoryRangeTest, ContentsEqual)
{
    int a = 0x12;
    int x = 0x12345678;
    int y = x;
    int z = x - 1;

    EXPECT_TRUE(ContentsEqual(infra::MakeByteRange(x), infra::MakeByteRange(y)));
    EXPECT_FALSE(ContentsEqual(infra::MakeByteRange(x), infra::MakeByteRange(z)));
    EXPECT_FALSE(ContentsEqual(infra::MakeByteRange(x), infra::MakeByteRange(a)));

    std::array<int, 2> a1 = { 1, 2 };
    std::array<int, 3> a2 = { 1, 2, 3 };
    EXPECT_FALSE(infra::ContentsEqual(infra::MakeByteRange(a1), infra::MakeByteRange(a2)));
}

TEST(MemoryRangeTest, IntersectingRange)
{
    std::array<uint8_t, 10> range;

    EXPECT_EQ(infra::ByteRange(range.data() + 2, range.data() + 4), infra::IntersectingRange(infra::ByteRange(range.data(), range.data() + 4), infra::ByteRange(range.data() + 2, range.data() + 6)));
    EXPECT_EQ(infra::ByteRange(range.data() + 2, range.data() + 2), infra::IntersectingRange(infra::ByteRange(range.data(), range.data() + 2), infra::ByteRange(range.data() + 2, range.data() + 6)));
    EXPECT_EQ(infra::ByteRange(), infra::IntersectingRange(infra::ByteRange(range.data(), range.data() + 2), infra::ByteRange(range.data() + 3, range.data() + 6)));
    EXPECT_EQ(infra::ByteRange(), infra::IntersectingRange(infra::ByteRange(range.data() + 3, range.data() + 6), infra::ByteRange(range.data(), range.data() + 2)));
}

TEST(MemoryRangeTest, Head)
{
    std::array<uint8_t, 10> range;

    EXPECT_EQ(infra::ByteRange(range.data(), range.data() + 4), infra::Head(infra::ByteRange(range), 4));
    EXPECT_EQ(infra::ByteRange(range.data(), range.data() + 10), infra::Head(infra::ByteRange(range), 14));
}

TEST(MemoryRangeTest, Tail)
{
    std::array<uint8_t, 10> range;

    EXPECT_EQ(infra::ByteRange(range.data() + 6, range.data() + 10), infra::Tail(infra::ByteRange(range), 4));
    EXPECT_EQ(infra::ByteRange(range.data(), range.data() + 10), infra::Tail(infra::ByteRange(range), 14));
}

TEST(MemoryRangeTest, DiscardHead)
{
    std::array<uint8_t, 10> range;

    EXPECT_EQ(infra::ByteRange(range.data() + 4, range.data() + range.size()), infra::DiscardHead(infra::ByteRange(range), 4));
    EXPECT_EQ(infra::ByteRange(), infra::DiscardHead(infra::ByteRange(range), 14));
}

TEST(MemoryRangeTest, DiscardTail)
{
    std::array<uint8_t, 10> range;

    EXPECT_EQ(infra::ByteRange(range.data(), range.data() + 6), infra::DiscardTail(infra::ByteRange(range), 4));
    EXPECT_EQ(infra::ByteRange(), infra::DiscardTail(infra::ByteRange(range), 14));
}

TEST(MemoryRangeTest, FindAndSplit)
{
    std::array<uint8_t, 4> range{ 1, 2, 3, 4 };
    infra::ConstByteRange first, second;
    std::tie(first, second) = infra::FindAndSplit(infra::MakeRange(range), 3);
    EXPECT_EQ(infra::ByteRange(range.data(), range.data() + 2), first);
    EXPECT_EQ(infra::ByteRange(range.data() + 2, range.data() + 4), second);
}

TEST(MemoryRangeTest, Convert)
{
    std::array<uint8_t, 4> range{ 1, 2, 3, 4 };

    EXPECT_EQ(0x04030201, infra::Convert<uint32_t>(infra::MakeRange(range)));
}

TEST(MemoryRangeTest, TestCompare)
{
    std::array<uint8_t, 3> range = { 1, 2, 3 };

    EXPECT_FALSE(infra::ByteRange(range.data(), range.data() + 2) == infra::ByteRange(range.data() + 1, range.data() + 2));
    EXPECT_FALSE(infra::ByteRange(range.data(), range.data() + 3) == infra::ByteRange(range.data(), range.data() + 2));
    EXPECT_TRUE(infra::ByteRange(range.data(), range.data() + 3) == infra::ByteRange(range.data(), range.data() + 3));
    EXPECT_FALSE(infra::ByteRange(range.data(), range.data() + 3) != infra::ByteRange(range.data(), range.data() + 3));

    EXPECT_TRUE(infra::ByteRange(range.data(), range.data() + 3) == range);
    EXPECT_TRUE(range == infra::ByteRange(range.data(), range.data() + 3));
    std::array<uint8_t, 3> otherRange = { 2, 3, 4 };
    EXPECT_FALSE(infra::ByteRange(range.data(), range.data() + 3) == otherRange);

    EXPECT_TRUE(infra::ByteRange(range.data(), range.data() + 3) != otherRange);
    EXPECT_TRUE(otherRange != infra::ByteRange(range.data(), range.data() + 3));
}

TEST(MemoryRangeTest, TestMakeRangeFromArray)
{
    uint8_t array[3] = { static_cast<uint8_t>(1), static_cast<uint8_t>(2), static_cast<uint8_t>(3) };
    const uint8_t constArray[3] = { static_cast<uint8_t>(1), static_cast<uint8_t>(2), static_cast<uint8_t>(3) };

    EXPECT_EQ(infra::ByteRange(reinterpret_cast<uint8_t*>(&array), reinterpret_cast<uint8_t*>(&array) + 3), infra::MakeRange(array));
    EXPECT_EQ(infra::ConstByteRange(reinterpret_cast<const uint8_t*>(&constArray), reinterpret_cast<const uint8_t*>(&constArray) + 3), infra::MakeRange(constArray));
}

TEST(MemoryRangeTest, TestMakeRangeFromContainer)
{
    infra::BoundedVector<uint8_t>::WithMaxSize<3> container({ static_cast<uint8_t>(1), static_cast<uint8_t>(2), static_cast<uint8_t>(3) });

    EXPECT_EQ(infra::ByteRange(&container.front(), &container.front() + 3), infra::MakeRange(container));
    EXPECT_EQ(infra::ByteRange(&container.front(), &container.front() + 3), infra::MakeRange(static_cast<const infra::BoundedVector<uint8_t>&>(container)));
}

TEST(MemoryRangeTest, TestMakeVectorFromRange)
{
    std::array<uint8_t, 3> array{ 2, 3, 4 };
    EXPECT_EQ((std::vector<uint8_t>{ 2, 3, 4 }), infra::MakeVector(infra::ConstByteRange(array)));
}

TEST(MemoryRangeTest, ConstCasts)
{
    std::array<uint8_t, 3> array{ 2, 3, 4 };

    EXPECT_EQ(array, infra::ConstCastByteRange(infra::MakeRange(array)));
    EXPECT_EQ(array, infra::ConstCastByteRange(infra::MakeConst(infra::MakeRange(array))));
}

TEST(MemoryRangeTest, MakeStringByteRange)
{
    const char* string = "string";
    std::string stdString = "string";

    EXPECT_EQ(infra::MemoryRange<const char>(string, string + std::strlen(string)), infra::ReinterpretCastMemoryRange<const char>(infra::MakeStringByteRange(string)));
    EXPECT_EQ(infra::MemoryRange<const char>(stdString.data(), stdString.data() + stdString.size()), infra::ReinterpretCastMemoryRange<const char>(infra::MakeStringByteRange(stdString)));
}
