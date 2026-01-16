#include "infra/util/PolymorphicVariant.hpp"
#include "gtest/gtest.h"

namespace
{
    class State
    {
    public:
        State() = default;
        State(const State&) = delete;
        State& operator=(const State&) = delete;
        virtual ~State() = default;

        virtual int Identifier() const = 0;
    };

    class State1
        : public State
    {
    public:
        int Identifier() const override
        {
            return identifier;
        }

    private:
        int identifier = 1;
    };

    class State2
        : public State
    {
    public:
        int Identifier() const override
        {
            return identifier;
        }

    private:
        int identifier = 2;
    };

    class CopyableState
    {
    public:
        virtual ~CopyableState() = default;

        virtual int Identifier() const = 0;
    };

    class CopyableState1
        : public CopyableState
    {
    public:
        int Identifier() const override
        {
            return identifier;
        }

    private:
        int identifier = 1;
    };

    class CopyableState2
        : public CopyableState
    {
    public:
        int Identifier() const override
        {
            return identifier;
        }

    private:
        int identifier = 2;
    };

    class ComparableState1
        : public State
    {
    public:
        bool operator==(const ComparableState1& other) const
        {
            return true;
        }

        bool operator<(const ComparableState1& other) const
        {
            return false;
        }

        int Identifier() const override
        {
            return identifier;
        }

    private:
        int identifier = 1;
    };

    class ComparableState2
        : public State
    {
    public:
        bool operator==(const ComparableState2& other) const
        {
            return true;
        }

        bool operator<(const ComparableState2& other) const
        {
            return false;
        }

        int Identifier() const override
        {
            return identifier;
        }

    private:
        int identifier = 2;
    };
}

TEST(PolymorphicVariantTest, after_default_construction_State1_is_created)
{
    infra::PolymorphicVariant<State, State1, State2> v;
    EXPECT_EQ(1, v->Identifier());
}

TEST(PolymorphicVariantTest, create_State1)
{
    infra::PolymorphicVariant<State, State1, State2> v((std::in_place_type_t<State1>()));
    EXPECT_EQ(1, v->Identifier());
}

TEST(PolymorphicVariantTest, create_State2)
{
    infra::PolymorphicVariant<State, State1, State2> v((std::in_place_type_t<State2>()));
    EXPECT_EQ(2, v->Identifier());
}

TEST(PolymorphicVariantTest, assign_State2_after_construction)
{
    infra::PolymorphicVariant<State, State1, State2> v;
    v.Emplace<State2>();
    EXPECT_EQ(2, v->Identifier());
}

TEST(PolymorphicVariantTest, Emplace_returns_reference_to_constructed_object)
{
    infra::PolymorphicVariant<State, State1, State2> v;

    State1& state1 = v.Emplace<State1>();
    EXPECT_EQ(1, state1.Identifier());
}

TEST(PolymorphicVariantTest, create_State1_by_copy)
{
    infra::PolymorphicVariant<CopyableState, CopyableState1, CopyableState2> v((CopyableState1()));
    EXPECT_EQ(1, v->Identifier());
}

TEST(PolymorphicVariantTest, create_State2_by_copy)
{
    infra::PolymorphicVariant<CopyableState, CopyableState1, CopyableState2> v((CopyableState2()));
    EXPECT_EQ(2, v->Identifier());
}

TEST(PolymorphicVariantTest, copy_construct_variant)
{
    infra::PolymorphicVariant<CopyableState, CopyableState1, CopyableState2> v1((CopyableState2()));
    infra::PolymorphicVariant<CopyableState, CopyableState1, CopyableState2> v2(v1);
    EXPECT_EQ(2, v2->Identifier());
}

TEST(PolymorphicVariantTest, copy_construct_from_narrower_variant)
{
    infra::PolymorphicVariant<CopyableState, CopyableState2> v1((CopyableState2()));
    infra::PolymorphicVariant<CopyableState, CopyableState1, CopyableState2> v2(v1);
    EXPECT_EQ(2, v2->Identifier());
}

TEST(PolymorphicVariantTest, copy_assign_variant)
{
    infra::PolymorphicVariant<CopyableState, CopyableState1, CopyableState2> v1;
    infra::PolymorphicVariant<CopyableState, CopyableState1, CopyableState2> v2((CopyableState2()));
    v1 = v2;
    EXPECT_EQ(2, v1->Identifier());
}

TEST(PolymorphicVariantTest, copy_assign_from_narrower_variant)
{
    infra::PolymorphicVariant<CopyableState, CopyableState1, CopyableState2> v1;
    infra::PolymorphicVariant<CopyableState, CopyableState2> v2((CopyableState2()));
    v1 = v2;
    EXPECT_EQ(2, v1->Identifier());
}

TEST(PolymorphicVariantTest, compare_same_states)
{
    infra::PolymorphicVariant<State, ComparableState1, ComparableState2> v1((std::in_place_type_t<ComparableState1>()));
    infra::PolymorphicVariant<State, ComparableState1, ComparableState2> v2((std::in_place_type_t<ComparableState1>()));
    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 != v2);
    EXPECT_FALSE(v1 < v2);
    EXPECT_FALSE(v1 > v2);
    EXPECT_TRUE(v1 <= v2);
    EXPECT_TRUE(v1 >= v2);
}

TEST(PolymorphicVariantTest, compare_different_states)
{
    infra::PolymorphicVariant<State, ComparableState1, ComparableState2> v1((std::in_place_type_t<ComparableState1>()));
    infra::PolymorphicVariant<State, ComparableState1, ComparableState2> v2((std::in_place_type_t<ComparableState2>()));
    EXPECT_FALSE(v1 == v2);
    EXPECT_TRUE(v1 != v2);
    EXPECT_TRUE(v1 < v2);
    EXPECT_FALSE(v1 > v2);
    EXPECT_TRUE(v1 <= v2);
    EXPECT_FALSE(v1 >= v2);
}

TEST(PolymorphicVariantTest, compare_same_states_by_value)
{
    infra::PolymorphicVariant<State, ComparableState1, ComparableState2> v((std::in_place_type_t<ComparableState1>()));
    ComparableState1 c;
    EXPECT_TRUE(v == c);
    EXPECT_FALSE(v != c);
    EXPECT_FALSE(v < c);
    EXPECT_FALSE(v > c);
    EXPECT_TRUE(v <= c);
    EXPECT_TRUE(v >= c);
}

TEST(PolymorphicVariantTest, compare_different_states_by_value)
{
    infra::PolymorphicVariant<State, ComparableState1, ComparableState2> v((std::in_place_type_t<ComparableState1>()));
    ComparableState2 c;
    EXPECT_FALSE(v == c);
    EXPECT_TRUE(v != c);
    EXPECT_TRUE(v < c);
    EXPECT_FALSE(v > c);
    EXPECT_TRUE(v <= c);
    EXPECT_FALSE(v >= c);
}
