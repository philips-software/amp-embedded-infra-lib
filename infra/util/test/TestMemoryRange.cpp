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
    EXPECT_EQ(0x04030201, infra::Convert<uint32_t>(infra::MakeConstRange(range)));
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
    EXPECT_EQ(infra::ConstByteRange(reinterpret_cast<const uint8_t*>(&constArray), reinterpret_cast<const uint8_t*>(&constArray) + 3), infra::MakeConstRange(constArray));
}

TEST(MemoryRangeTest, TestMakeRangeFromVector)
{
    std::vector<uint8_t> container({ static_cast<uint8_t>(1), static_cast<uint8_t>(2), static_cast<uint8_t>(3) });

    EXPECT_EQ(infra::ByteRange(&container.front(), &container.front() + 3), infra::MakeRange(container));
    EXPECT_EQ(infra::ByteRange(&container.front(), &container.front() + 3), infra::MakeRange(static_cast<const std::vector<uint8_t>&>(container)));
    EXPECT_EQ(infra::ByteRange(&container.front(), &container.front() + 3), infra::MakeConstRange(static_cast<const std::vector<uint8_t>&>(container)));
}

TEST(MemoryRangeTest, TestMakeRangeFromContainer)
{
    infra::BoundedVector<uint8_t>::WithMaxSize<3> container({ static_cast<uint8_t>(1), static_cast<uint8_t>(2), static_cast<uint8_t>(3) });

    EXPECT_EQ(infra::ByteRange(&container.front(), &container.front() + 3), infra::MakeRange(container));
    EXPECT_EQ(infra::ByteRange(&container.front(), &container.front() + 3), infra::MakeRange(static_cast<const infra::BoundedVector<uint8_t>&>(container)));
    EXPECT_EQ(infra::ByteRange(&container.front(), &container.front() + 3), infra::MakeConstRange(static_cast<const infra::BoundedVector<uint8_t>&>(container)));
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

TEST(MemoryRangeTest, ConstexprConstruction)
{
    {
        // MemoryRange()
        constexpr infra::MemoryRange<uint8_t> range;

        static_assert(range.empty(), "Range should be empty");
        static_assert(range.size() == 0, "Range size should be 0");
    }

    {
        // MemoryRange(T* begin, T* end)
        static constexpr uint8_t array[]{ 0, 1, 2 };
        constexpr infra::MemoryRange<const uint8_t> range{ &array[0], &array[0] + 3 };

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 3, "Range size should be 3");
        static_assert(range.begin() == &array[0], "Range begin should point to first element");
    }

    {
        // MemoryRange(const MemoryRange<U>& other)
        static constexpr uint8_t data[]{ 0, 1, 2 };
        constexpr infra::MemoryRange<const uint8_t> other{ &data[0], &data[0] + 3 };
        constexpr auto range{ other };

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 3, "Range size should be 3");
        static_assert(range.begin() == &data[0], "Range begin should point to first element");
    }

    {
        // MemoryRange(const std::array<T2, N>& array)
        static constexpr const std::array<uint8_t, 3> array{ 0, 1, 2 };
        constexpr infra::MemoryRange<const uint8_t> range{ array };

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 3, "Range size should be 3");
        static_assert(range.begin() == &array[0], "Range begin should point to first element");
    }

#if __cplusplus >= 202002L
    {
        // MemoryRange(const std::vector<T2>& vector) cannot be tested with C++17
        constexpr std::vector<uint8_t> data{};
        constexpr infra::MemoryRange<const uint8_t> range{ data };

        static_assert(range.empty(), "Range should be empty");
        static_assert(range.size() == 0, "Range size should be 0");
    }
#endif
}

TEST(MemoryRangeTest, ConstexprIteration)
{
    static constexpr std::array data{ 10, 20, 30, 40, 50 };
    constexpr infra::MemoryRange<const int> range{ data };

    constexpr int sum = [](infra::MemoryRange<const int> r)
    {
        int result = 0;
        for (const auto& element : r)
            result += element;
        return result;
    }(range);

    static_assert(sum == 150, "Sum should be 150");
}

TEST(MemoryRangeTest, ConstexprCompare)
{
    static constexpr std::array data1{ 10, 20, 30 };
    static constexpr std::array data3{ 10, 20, 31 };

    constexpr infra::MemoryRange<const int> range1{ data1 };
    constexpr infra::MemoryRange<const int> range2{ data1 };
    constexpr infra::MemoryRange<const int> range3{ data3 };

    static_assert(range1 == range2, "Ranges should be equal");
    static_assert(!(range1 != range2), "Ranges should be equal");
    static_assert(range1 != range3, "Ranges should not be equal");
    static_assert(!(range1 == range3), "Ranges should not be equal");
}

TEST(MemoryRangeTest, ConstexprAccess)
{
    static constexpr std::array data{ 10, 20, 30, 40, 50 };
    constexpr infra::MemoryRange<const int> range{ data };

    static_assert(range[0] == 10, "Element 0 should be 10");
    static_assert(range[1] == 20, "Element 1 should be 20");
    static_assert(range[2] == 30, "Element 2 should be 30");
    static_assert(range.front() == 10, "Front element should be 10");
    static_assert(range.back() == 50, "Back element should be 50");
    static_assert(range.contains(&data[2]), "Range should contain element at index 2");
}

TEST(MemoryRangeTest, ConstexprClear)
{
    static constexpr std::array data{ 10, 20, 30 };
    constexpr infra::MemoryRange<const int> range{ data };

    constexpr infra::MemoryRange<const int> clearedRange = [](infra::MemoryRange<const int> r)
    {
        r.clear();
        return r;
    }(range);

    static_assert(!range.empty(), "Original range should be unaffected");
    static_assert(clearedRange.empty(), "Cleared range should be empty");
}

TEST(MemoryRangeTest, ConstexprPop)
{
    static constexpr std::array data{ 10, 20, 30, 40, 50 };
    constexpr infra::MemoryRange<const int> range{ data };

    constexpr infra::MemoryRange<const int> poppedFrontRange = [](infra::MemoryRange<const int> r)
    {
        r.pop_front(2);
        return r;
    }(range);

    static_assert(poppedFrontRange.size() == 3, "Popped front range size should be 3");
    static_assert(poppedFrontRange.front() == 30, "Popped front range front should be 30");

    constexpr infra::MemoryRange<const int> poppedBackRange = [](infra::MemoryRange<const int> r)
    {
        r.pop_back(2);
        return r;
    }(range);

    static_assert(poppedBackRange.size() == 3, "Popped back range size should be 3");
    static_assert(poppedBackRange.back() == 30, "Popped back range back should be 30");

    static_assert(range.size() == 5, "Original range should be unaffected");
}

TEST(MemoryRangeTest, ConstexprShrink)
{
    static constexpr std::array data{ 10, 20, 30, 40, 50 };
    constexpr infra::MemoryRange<const int> range{ data };

    constexpr infra::MemoryRange<const int> shrunkFrontRange = [](infra::MemoryRange<const int> r)
    {
        r.shrink_from_front_to(2);
        return r;
    }(range);

    static_assert(shrunkFrontRange.size() == 2, "Shrunk front range size should be 2");
    static_assert(shrunkFrontRange.front() == 40, "Shrunk front range front should be 40");

    constexpr infra::MemoryRange<const int> shrunkBackRange = [](infra::MemoryRange<const int> r)
    {
        r.shrink_from_back_to(3);
        return r;
    }(range);

    static_assert(shrunkBackRange.size() == 3, "Shrunk back range size should be 3");
    static_assert(shrunkBackRange.back() == 30, "Shrunk back range back should be 30");

    static_assert(range.size() == 5, "Original range should be unaffected");
}

TEST(MemoryRangeTest, ConstexprMakeRange)
{
    {
        // MakeRange(const T* b, const T* e)
        static constexpr std::array data{ 10, 20, 30 };
        constexpr auto range = infra::MakeRange(&data[0], &data[0] + 3);

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 3, "Range size should be 3");
        static_assert(range.begin() == &data[0], "Range begin should point to first element");
    }

    {
        // MakeRange(const T (&data)[N])
        static constexpr uint8_t data[] = { 10, 20, 30 };
        constexpr auto range = infra::MakeRange(data);

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 3, "Range size should be 3");
        static_assert(range.begin() == &data[0], "Range begin should point to first element");
    }

    {
        // MakeConstRange(const T (&data)[N])
        static constexpr uint8_t data[] = { 10, 20, 30 };
        constexpr auto range = infra::MakeConstRange(data);

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 3, "Range size should be 3");
        static_assert(range.begin() == &data[0], "Range begin should point to first element");
    }

    {
        // MakeRange(const std::array<T, N>& container)
        static constexpr std::array data{ 10, 20, 30 };
        constexpr auto range = infra::MakeRange(data);

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 3, "Range size should be 3");
        static_assert(range.begin() == &data[0], "Range begin should point to first element");
    }

    {
        // MakeConstRange(const std::array<T, N>& container)
        static constexpr std::array data{ 10, 20, 30 };
        constexpr auto range = infra::MakeConstRange(data);

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 3, "Range size should be 3");
        static_assert(range.begin() == &data[0], "Range begin should point to first element");
    }

#if __cplusplus >= 202002L
    {
        // MakeRange(const std::vector<T>& range) cannot be tested with C++17
        constexpr std::vector<int> data{};
        constexpr auto range = infra::MakeRange(data);

        static_assert(range.empty(), "Range should be empty");
        static_assert(range.size() == 0, "Range size should be 0");
    }

    {
        // MakeConstRange(const std::vector<T>& range) cannot be tested with C++17
        constexpr std::vector<int> data{};
        constexpr auto range = infra::MakeConstRange(data);

        static_assert(range.empty(), "Range should be empty");
        static_assert(range.size() == 0, "Range size should be 0");
    }
#endif

    {
        // MakeRangeFromSingleObject(T& object)
        static constexpr int object = 42;
        constexpr auto range = infra::MakeRangeFromSingleObject(object);

        static_assert(!range.empty(), "Range should not be empty");
        static_assert(range.size() == 1, "Range size should be 1");
        static_assert(range.front() == 42, "Range front should be 42");
    }
}

TEST(MemoryRangeTest, ConstexprContentsEqual)
{
#if __cplusplus >= 202002L
    static constexpr std::array data1{ 1, 2, 3 };
    static constexpr std::array data2{ 1, 2, 3 };
    static constexpr std::array data3{ 1, 2, 4 };

    constexpr bool equal12 = infra::ContentsEqual(infra::MakeConstRange(data1), infra::MakeConstRange(data2));
    constexpr bool equal13 = infra::ContentsEqual(infra::MakeConstRange(data1), infra::MakeConstRange(data3));

    static_assert(equal12, "Ranges should be equal");
    static_assert(!equal13, "Ranges should not be equal");
#endif
}
