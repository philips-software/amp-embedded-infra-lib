#include "infra/util/StaticStorage.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(StaticStorageTest, TestConstruction)
{
    infra::StaticStorage<bool> s;
    (void)s;
}

TEST(StaticStorageTest, TestConstructionOfValue)
{
    infra::StaticStorage<bool> s;
    s.Construct(true);
    EXPECT_TRUE(*s);
}

TEST(StaticStorageTest, TestDestruction)
{
    static infra::MockCallback<void()> constructor;
    static infra::MockCallback<void()> destructor;

    struct X
    {
        X() { constructor.callback(); }
        ~X() { destructor.callback(); }
    };

    infra::StaticStorage<X> s;

    EXPECT_CALL(constructor, callback());
    EXPECT_CALL(destructor, callback());

    s.Construct();
    s.Destruct();
}

TEST(StaticStorageTest, TestInheritanceTree)
{
    struct A
    {
        virtual ~A() = default;
    };

    struct B
        : A
    {};

    struct C
        : A
    {
        C(uint64_t aX)
            : x(aX)
        {}

        uint64_t x;
    };

    infra::StaticStorageForInheritanceTree<A, B> b;
    infra::StaticStorageForInheritanceTree<A, B, C> c;

    EXPECT_EQ(sizeof(b), 2 * sizeof(void*));
    EXPECT_EQ(sizeof(c), 3 * sizeof(uint64_t));

    b.Construct<B>();
    c.Construct<C>(5);

    b.Destruct();
    c.Destruct();
}
