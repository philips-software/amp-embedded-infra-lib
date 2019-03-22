#include "gtest/gtest.h"
#include "infra/util/Variant.hpp"
#include <cstdint>

TEST(VariantTest, TestEmptyConstruction)
{
    infra::Variant<bool> v;
}

TEST(VariantTest, TestConstructionWithBool)
{
    bool b;
    infra::Variant<bool> v(b);
}

TEST(VariantTest, TestConstructionWithVariant)
{
    infra::Variant<bool, int> i(5);
    infra::Variant<bool, int> v(i);
    EXPECT_EQ(5, v.Get<int>());
}

TEST(VariantTest, TestConstructionWithNarrowVariant)
{
    infra::Variant<int> i(5);
    infra::Variant<bool, int> v(i);
    EXPECT_EQ(5, v.Get<int>());
}

TEST(VariantTest, TestConstructionAtIndex)
{
    infra::Variant<uint8_t, uint16_t> v(infra::atIndex, 1, 3);
    EXPECT_EQ(1, v.Which());
    EXPECT_EQ(3, v.Get<uint16_t>());
}

TEST(VariantTest, TestGetBool)
{
    bool b = true;
    infra::Variant<bool> v(b);
    EXPECT_TRUE(v.Get<bool>());
}

TEST(VariantTest, TestAssignment)
{
    infra::Variant<bool, int> v(true);
    v = 5;
    EXPECT_EQ(5, v.Get<int>());
}

TEST(VariantTest, TestInPlaceConstruction)
{
    struct MyStruct
    {
        MyStruct(int aX, int aY): x(aX), y(aY) {};

        int x;
        int y;
    };

    infra::Variant<MyStruct> v(infra::InPlaceType<MyStruct>(), 2, 3);
    EXPECT_EQ(0, v.Which());
    EXPECT_EQ(2, v.Get<MyStruct>().x);
    EXPECT_EQ(3, v.Get<MyStruct>().y);
}

TEST(VariantTest, TestEmplace)
{
    struct MyStruct
    {
        MyStruct(int aX, int aY): x(aX), y(aY) {};

        bool operator==(const MyStruct& other) const
        {
            return x == other.x && y == other.y;
        }

        int x;
        int y;
    };

    infra::Variant<bool, MyStruct> v(true);
    EXPECT_EQ(MyStruct(2, 3), v.Emplace<MyStruct>(2, 3));
    EXPECT_EQ(1, v.Which());
    EXPECT_EQ(2, v.Get<MyStruct>().x);
    EXPECT_EQ(3, v.Get<MyStruct>().y);
}

TEST(VariantTest, TestAssignmentFromVariant)
{
    infra::Variant<bool, int> v(true);
    v = infra::Variant<bool, int>(5);
    EXPECT_EQ(5, v.Get<int>());
}

TEST(VariantTest, TestSelfAssignment)
{
    infra::Variant<bool, int> v(true);
    v = v;
    EXPECT_EQ(true, v.Get<bool>());
}

TEST(VariantTest, TestAssignmentFromNarrowVariant)
{
    infra::Variant<bool, int> v(true);
    v = infra::Variant<int>(5);
    EXPECT_EQ(5, v.Get<int>());
}

TEST(VariantTest, TestVariantWithTwoTypes)
{
    int i = 5;
    infra::Variant<bool, int> v(i);
    EXPECT_EQ(5, v.Get<int>());
}

TEST(VariantTest, TestVisitor)
{
    struct Visitor
        : infra::StaticVisitor<void>
    {
        Visitor()
            : passed(0)
        {}

        void operator()(bool b)
        {}

        void operator()(int i)
        {
            ++passed;
        }

        int passed;
    };

    int i = 5;
    infra::Variant<bool, int> variant(i);
    Visitor visitor;
    infra::ApplyVisitor(visitor, variant);
    EXPECT_EQ(1, visitor.passed);
}

TEST(VariantTest, TestReturningVisitor)
{
    struct Visitor
        : infra::StaticVisitor<bool>
    {
        bool operator()(bool b)
        {
            return false;
        }

        bool operator()(int i)
        {
            return true;
        }
    };

    int i = 5;
    infra::Variant<bool, int> variant(i);
    Visitor visitor;
    EXPECT_EQ(true, infra::ApplyVisitor(visitor, variant));
}

TEST(VariantTest, TestEqual)
{
    int i = 1;
    int j = 2;
    bool k = true;
    const infra::Variant<bool, int> v1(i);
    const infra::Variant<bool, int> v2(i);
    const infra::Variant<bool, int> v3(j);
    const infra::Variant<bool, int> v4(k);

    EXPECT_EQ(v1, v2);
    EXPECT_NE(v1, v3);
    EXPECT_NE(v1, v4);
}

TEST(VariantTest, TestLessThan)
{
    int i = 1;
    int j = 2;
    bool k = true;
    const infra::Variant<bool, int> v1(i);
    const infra::Variant<bool, int> v2(i);
    const infra::Variant<bool, int> v3(j);
    const infra::Variant<bool, int> v4(k);

    EXPECT_GE(v1, v2);
    EXPECT_LE(v1, v2);
    EXPECT_LT(v1, v3);
    EXPECT_GT(v1, v4);
}

TEST(VariantTest, TestEqualValue)
{
    int i = 1;
    int j = 2;
    bool k = true;
    const infra::Variant<bool, int> v1(i);

    EXPECT_EQ(v1, i);
    EXPECT_NE(v1, j);
    EXPECT_NE(v1, k);
}

TEST(VariantTest, TestLessThanValue)
{
    int i = 1;
    int j = 2;
    bool k = true;
    const infra::Variant<bool, int> v1(i);

    EXPECT_GE(v1, i);
    EXPECT_LE(v1, i);
    EXPECT_LT(v1, j);
    EXPECT_GT(v1, k);
}

struct DoubleVisitor
    : infra::StaticVisitor<int>
{
    template<class T, class U>
    int operator()(T x, U y)
    {
        return x + y;
    }
};

TEST(VariantTest, TestDoubleVisitor)
{
    infra::Variant<uint32_t, int> variant1(infra::InPlaceType<int>(), 5);
    infra::Variant<uint32_t, int> variant2(infra::InPlaceType<uint32_t>(), 1u);
    DoubleVisitor visitor;
    EXPECT_EQ(2, infra::ApplyVisitor(visitor, variant2, variant2));
    EXPECT_EQ(6, infra::ApplyVisitor(visitor, variant1, variant2));
    EXPECT_EQ(10, infra::ApplyVisitor(visitor, variant1, variant1));
}

struct EmptyVisitor
    : infra::StaticVisitor<void>
{
    template<class T>
    void operator()(T)
    {}

    template<class T>
    void operator()(T, T)
    {}

    template<class T, class U>
    void operator()(T, U)
    {}
};

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};
struct F {};

TEST(VariantTest, TestRecursiveLoopUnrolling3_1)
{
    infra::Variant<A, B, C> v((A()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling3_2)
{
    infra::Variant<A, B, C> v((B()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling3_3)
{
    infra::Variant<A, B, C> v((C()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling4_1)
{
    infra::Variant<A, B, C, D> v((A()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling4_2)
{
    infra::Variant<A, B, C, D> v((B()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling4_3)
{
    infra::Variant<A, B, C, D> v((C()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling4_4)
{
    infra::Variant<A, B, C, D> v((D()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling5_1)
{
    infra::Variant<A, B, C, D, E> v((A()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling5_2)
{
    infra::Variant<A, B, C, D, E> v((B()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling5_3)
{
    infra::Variant<A, B, C, D, E> v((C()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling5_4)
{
    infra::Variant<A, B, C, D, E> v((D()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrolling5_5)
{
    infra::Variant<A, B, C, D, E> v((E()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);
}

TEST(VariantTest, TestRecursiveLoopUnrollingX)
{
    infra::Variant<A, B, C, D, E, F> v((F()));
    EmptyVisitor visitor;
    infra::ApplyVisitor(visitor, v);
    infra::ApplyVisitor(visitor, v, v);
    infra::ApplySameTypeVisitor(visitor, v, v);    
}
