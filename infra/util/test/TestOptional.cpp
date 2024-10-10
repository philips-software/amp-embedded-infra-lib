#include "infra/util/Optional.hpp"
#include "gtest/gtest.h"

namespace infra
{
    void OptionalCoverageDummy();
}

TEST(OptionalTest, OptionalCoverageDummy)
{
    infra::OptionalCoverageDummy();
}

TEST(OptionalTest, TestTransformOptional)
{
    infra::Optional<bool> o = std::make_optional(true);
    infra::Optional<bool> empty;
    EXPECT_EQ(std::make_optional(5), infra::TransformOptional(std::make_optional(true), [](bool value)
                                         {
                                             return 5;
                                         }));
    EXPECT_EQ(infra::none, infra::TransformOptional(empty, [](bool value)
                               {
                                   return 5;
                               }));
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
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    EXPECT_FALSE(o);
    EXPECT_TRUE(!o);
}

TEST(OptionalForPolymorphicObjectsTest, TestConstructedEmptyWithNone)
{
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(infra::none);
    EXPECT_FALSE(static_cast<bool>(o));
}

TEST(OptionalForPolymorphicObjectsTest, TestConstructedWithValue)
{
    bool value(true);
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(infra::InPlaceType<PolymorphicBool>(), value);
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestConstructedWithMovedValue)
{
    bool value(true);
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(infra::InPlaceType<PolymorphicBool>(), std::move(value));
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestConstructedWithDescendant)
{
    infra::OptionalForPolymorphicObjects<PolymorphicBoolDescendant, 0> descendant(infra::InPlaceType<PolymorphicBoolDescendant>(), true);
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(descendant);
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);

    infra::OptionalForPolymorphicObjects<PolymorphicBoolDescendant, 0> emptyDescendant;
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> x(emptyDescendant);
    EXPECT_FALSE(static_cast<bool>(x));
}

TEST(OptionalForPolymorphicObjectsTest, TestAssignLValue)
{
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    bool b = true;
    o = b;
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestAssignRValue)
{
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    o = true;
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);
}

TEST(OptionalForPolymorphicObjectsTest, TestAssignNone)
{
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o(infra::InPlaceType<PolymorphicBool>(), true);
    o = infra::none;
    EXPECT_FALSE(static_cast<bool>(o));
}

TEST(OptionalForPolymorphicObjectsTest, TestAssignDescendant)
{
    infra::OptionalForPolymorphicObjects<PolymorphicBoolDescendant, 0> descendant(infra::InPlaceType<PolymorphicBoolDescendant>(), true);
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    o = descendant;
    EXPECT_TRUE(static_cast<bool>(o));
    EXPECT_TRUE(*o);

    infra::OptionalForPolymorphicObjects<PolymorphicBoolDescendant, 0> emptyDescendant;
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> x;
    x = emptyDescendant;
    EXPECT_FALSE(static_cast<bool>(x));
}

TEST(OptionalForPolymorphicObjectsTest, TestCompareToNone)
{
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
    EXPECT_TRUE(o == infra::none);
    EXPECT_FALSE(o != infra::none);
    EXPECT_TRUE(infra::none == o);
    EXPECT_FALSE(infra::none != o);
}

TEST(OptionalForPolymorphicObjectsTest, TestCompare)
{
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> t(infra::InPlaceType<PolymorphicBool>(), true);
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> f(infra::InPlaceType<PolymorphicBool>(), false);
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> u;

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
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> t(infra::InPlaceType<PolymorphicBool>(), true);
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> u;

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
    infra::OptionalForPolymorphicObjects<PolymorphicBool, sizeof(void*)> o;
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

    infra::OptionalForPolymorphicObjects<X, sizeof(void*)> o{ infra::InPlaceType<X>(), X() };
    EXPECT_TRUE(o->x);
    EXPECT_TRUE((const_cast<const infra::OptionalForPolymorphicObjects<X, sizeof(void*)>&>(o)->x));
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

    infra::OptionalForPolymorphicObjects<Base, sizeof(void*)> optionalBase((infra::InPlaceType<Derived>()));
    optionalBase.emplace<Derived>();
}
