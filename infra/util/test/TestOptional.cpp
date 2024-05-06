#include "infra/util/Optional.hpp"
#include "gtest/gtest.h"

namespace infra
{
    void OptionalCoverageDummy();
}

TEST(OptionalTest, OptionalCoverageDummy)
{
    std::optionalCoverageDummy();
}

TEST(OptionalTest, TestConstructedEmpty)
{
    std::optional<bool> o;
    EXPECT_FALSE(o);
    EXPECT_TRUE(!o);
}

TEST(OptionalTest, TestConstructedEmptyWithNone)
{
    std::optional<bool> o(std::nullopt);
    EXPECT_FALSE(o);
}

TEST(OptionalTest, TestConstructedWithValue)
{
    bool value(true);
    std::optional<bool> o(std::in_place, value);
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
    EXPECT_TRUE(*(o.operator->()));
}

TEST(OptionalTest, TestConstructedWithMovedValue)
{
    bool value(true);
    std::optional<bool> o(std::in_place, std::move(value));
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalTest, TestConstructedWithConvertibleValue)
{
    std::optional<short int> x(std::in_place, 1);
    std::optional<int> y(x);
    EXPECT_TRUE(static_cast<bool>(y));
    EXPECT_EQ(1, *y);

    EXPECT_FALSE(std::optional<int>(std::optional<short int>()));
}

TEST(OptionalTest, TestCopyConstruction)
{
    std::optional<bool> o1(std::in_place, true);
    std::optional<bool> o2(o1);
    EXPECT_TRUE(static_cast<bool>(o1));
    EXPECT_TRUE(static_cast<bool>(o2));
    EXPECT_TRUE(*o2);
}

TEST(OptionalTest, TestMoveConstruction)
{
    std::optional<bool> o1(std::in_place, true);
    std::optional<bool> o2(std::move(o1));
    EXPECT_FALSE(o1);
    EXPECT_TRUE(static_cast<bool>(o2));
    EXPECT_TRUE(*o2);
}

TEST(OptionalTest, TestMoveConstructedWithEmptyOptional)
{
    std::optional<bool> e;
    std::optional<bool> o(std::move(e));
    EXPECT_FALSE(static_cast<bool>(o));
}

TEST(OptionalTest, TestCopyAssign)
{
    std::optional<bool> o1(std::in_place, true);
    std::optional<bool> o2;
    o2 = o1;
    EXPECT_TRUE(static_cast<bool>(o1));
    EXPECT_TRUE(static_cast<bool>(o2));
    EXPECT_TRUE(*o2);
}

TEST(OptionalTest, TestCopyAssignWithSelf)
{
    std::optional<bool> o1(std::in_place, true);
    o1 = o1;
    EXPECT_TRUE(static_cast<bool>(o1));
    EXPECT_TRUE(*o1);
}

TEST(OptionalTest, TestCopyAssignWithEmptyOptional)
{
    std::optional<bool> o1;
    std::optional<bool> o2;
    o2 = o1;
    EXPECT_FALSE(static_cast<bool>(o2));
}

TEST(OptionalTest, TestMoveAssign)
{
    std::optional<bool> o1(std::in_place, true);
    std::optional<bool> o2;
    o2 = std::move(o1);
    EXPECT_FALSE(static_cast<bool>(o1));
    EXPECT_TRUE(static_cast<bool>(o2));
    EXPECT_TRUE(*o2);
}

TEST(OptionalTest, TestMoveAssignWithEmpty)
{
    std::optional<bool> o1;
    std::optional<bool> o2;
    o2 = std::move(o1);
    EXPECT_FALSE(static_cast<bool>(o1));
    EXPECT_FALSE(static_cast<bool>(o2));
}

TEST(OptionalTest, TestAssignValue)
{
    std::optional<bool> o;
    o = true;
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalTest, TestAssignNone)
{
    std::optional<bool> o(std::in_place, true);
    o = std::nullopt;
    EXPECT_FALSE(o);
}

TEST(OptionalTest, TestCompareToNone)
{
    std::optional<bool> o;
    EXPECT_TRUE(o == std::nullopt);
    EXPECT_FALSE(o != std::nullopt);
    EXPECT_TRUE(std::nullopt == o);
    EXPECT_FALSE(std::nullopt != o);
}

TEST(OptionalTest, TestCompare)
{
    std::optional<bool> t(std::in_place, true);
    std::optional<bool> f(std::in_place, false);
    std::optional<bool> u;

    EXPECT_TRUE(t == t);
    EXPECT_TRUE(f == f);
    EXPECT_TRUE(u == u);

    EXPECT_FALSE(t == f);
    EXPECT_FALSE(t == u);

    EXPECT_FALSE(f == u);

    EXPECT_FALSE(t != t);
}

TEST(OptionalTest, TestCompareToValue)
{
    std::optional<bool> t(std::in_place, true);
    std::optional<bool> u;

    EXPECT_TRUE(t == true);
    EXPECT_TRUE(true == t);
    EXPECT_FALSE(t == false);

    EXPECT_FALSE(t != true);
    EXPECT_FALSE(true != t);

    EXPECT_FALSE(u == true);
}

TEST(OptionalTest, TestConstruct)
{
    bool b(true);
    std::optional<bool> o;
    o.emplace(std::move(b));
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalTest, TestConstructWithInitializerList)
{
    struct X
    {
        X(std::initializer_list<bool>)
        {}
    };

    std::optional<X> o;
    o.emplace({ true });
    EXPECT_TRUE(static_cast<bool>(o));
}

TEST(OptionalTest, TestIndirect)
{
    struct X
    {
        X()
            : x(true)
        {}

        bool x;
    };

    std::optional<X> o(std::in_place, X());
    EXPECT_TRUE(o->x);
    EXPECT_TRUE(const_cast<std::optional<X>&>(o)->x);
}

TEST(OptionalTest, TestMakeOptional)
{
    std::optional<bool> o = std::make_optional(true);
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalTest, TestTransformOptional)
{
    std::optional<bool> o = std::make_optional(true);
    std::optional<bool> empty;
    EXPECT_EQ(std::make_optional(5), infra::TransformOptional(std::make_optional(true), [](bool value)
                                         {
                                             return 5;
                                         }));
    EXPECT_EQ(std::nullopt, infra::TransformOptional(empty, [](bool value)
                                {
                                    return 5;
                                }));
}

TEST(OptionalTest, ValueOrGivesStoredWhenAvailable)
{
    std::optional<int> i(std::in_place, 5);
    EXPECT_EQ(5, i.value_or(2));
}

TEST(OptionalTest, ValueOrGivesParameterWhenNothingIsStored)
{
    std::optional<int> i;
    EXPECT_EQ(2, i.value_or(2));
}

TEST(OptionalTest, ValueOrConstRefGivesStoredWhenAvailable)
{
    std::optional<int> i(std::in_place, 5);
    const int x = 2;
    EXPECT_EQ(5, i.value_or(x));
}

TEST(OptionalTest, ValueOrConstRefGivesParameterWhenNothingIsStored)
{
    std::optional<int> i;
    const int x = 2;
    EXPECT_EQ(2, i.value_or(x));
}

TEST(OptionalTest, ValueOrDefaultGivesStoredWhenAvailable)
{
    std::optional<int> i(std::in_place, 5);
    EXPECT_EQ(5, i.ValueOrDefault());
}

TEST(OptionalTest, ValueOrDefaultGivesDefaultWhenNothingIsStored)
{
    std::optional<int> i;
    EXPECT_EQ(0, i.ValueOrDefault());
}

struct PolymorphicBool
{
    PolymorphicBool(bool value)
        : value(value)
    {}

    virtual ~PolymorphicBool() = default;

    operator bool() const
    {
        return value;
    }

    bool value;
};

struct PolymorphicBoolDescendant
    : PolymorphicBool
{
    using PolymorphicBool::PolymorphicBool;

    int additional;
};

TEST(OptionalForPolymorphicObjectsTest, TestConstructedEmpty)
{
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    EXPECT_FALSE(o);
    EXPECT_TRUE(!o);
}

TEST(OptionalForPolymorphicObjectsTest, TestConstructedEmptyWithNone)
{
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(std::nullopt);
    EXPECT_FALSE(static_cast<bool>(o));
}

TEST(OptionalForPolymorphicObjectsTest, TestConstructedWithValue)
{
    bool value(true);
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(std::in_place_type_t<PolymorphicBool>(), value);
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestConstructedWithMovedValue)
{
    bool value(true);
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(std::in_place_type_t<PolymorphicBool>(), std::move(value));
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestConstructedWithDescendant)
{
    std::optionalForPolymorphicObjects<PolymorphicBoolDescendant, 0> descendant(std::in_place_type_t<PolymorphicBoolDescendant>(), true);
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(descendant);
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);

    std::optionalForPolymorphicObjects<PolymorphicBoolDescendant, 0> emptyDescendant;
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> x(emptyDescendant);
    EXPECT_FALSE(static_cast<bool>(x));
}

TEST(OptionalForPolymorphicObjectsTest, TestAssignLValue)
{
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    bool b = true;
    o = b;
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestAssignRValue)
{
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    o = true;
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestAssignNone)
{
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(std::in_place_type_t<PolymorphicBool>(), true);
    o = std::nullopt;
    EXPECT_FALSE(static_cast<bool>(o));
}

TEST(OptionalForPolymorphicObjectsTest, TestAssignDescendant)
{
    std::optionalForPolymorphicObjects<PolymorphicBoolDescendant, 0> descendant(std::in_place_type_t<PolymorphicBoolDescendant>(), true);
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    o = descendant;
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);

    std::optionalForPolymorphicObjects<PolymorphicBoolDescendant, 0> emptyDescendant;
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> x;
    x = emptyDescendant;
    EXPECT_FALSE(static_cast<bool>(x));
}

TEST(OptionalForPolymorphicObjectsTest, TestCompareToNone)
{
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    EXPECT_TRUE(o == std::nullopt);
    EXPECT_FALSE(o != std::nullopt);
    EXPECT_TRUE(std::nullopt == o);
    EXPECT_FALSE(std::nullopt != o);
}

TEST(OptionalForPolymorphicObjectsTest, TestCompare)
{
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> t(std::in_place_type_t<PolymorphicBool>(), true);
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> f(std::in_place_type_t<PolymorphicBool>(), false);
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> u;

    EXPECT_TRUE(t == t);
    EXPECT_TRUE(f == f);
    EXPECT_TRUE(u == u);

    EXPECT_FALSE(t == f);
    EXPECT_FALSE(t == u);

    EXPECT_FALSE(f == u);
    EXPECT_TRUE(f != u);
}

TEST(OptionalForPolymorphicObjectsTest, TestCompareToValue)
{
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> t(std::in_place_type_t<PolymorphicBool>(), true);
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> u;

    EXPECT_TRUE(t == true);
    EXPECT_TRUE(true == t);
    EXPECT_FALSE(t == false);

    EXPECT_FALSE(t != true);
    EXPECT_FALSE(true != t);

    EXPECT_FALSE(u == true);
}

TEST(OptionalForPolymorphicObjectsTest, TestConstruct)
{
    PolymorphicBool b(true);
    std::optionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    o.emplace<PolymorphicBool>(std::move(b));
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestIndirect)
{
    struct X
    {
        X()
            : x(true)
        {}

        virtual ~X() = default;

        bool x;
    };

    std::optionalForPolymorphicObjects<X, sizeof(void*)> o{ std::in_place_type_t<X>(), X() };
    EXPECT_TRUE(o->x);
    EXPECT_TRUE((const_cast<const std::optionalForPolymorphicObjects<X, sizeof(void*)>&>(o)->x));
}

TEST(OptionalForPolymorphicObjectsTest, ConstructDescendant)
{
    struct Base
    {
        virtual ~Base() = default;
    };

    struct Derived
        : Base
    {
        int ExtraStorage;
    };

    std::optionalForPolymorphicObjects<Base, sizeof(void*)> optionalBase((std::in_place_type_t<Derived>()));
    optionalBase.emplace<Derived>();
}
