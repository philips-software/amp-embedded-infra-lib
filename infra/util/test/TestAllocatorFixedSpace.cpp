#include "gtest/gtest.h"
#include "infra/util/AllocatorFixedSpace.hpp"

class FixedSpaceAllocatorTest
    : public testing::Test
{
public:
    struct TestObject
    {};

    infra::AllocatorFixedSpace<TestObject, 1, void()> allocator;
};

TEST_F(FixedSpaceAllocatorTest, allocate_one_object)
{
    infra::UniquePtr<TestObject> object = allocator.Allocate();

    EXPECT_NE(nullptr, object);
}

TEST_F(FixedSpaceAllocatorTest, after_object_goes_out_of_scope_object_is_released)
{
    {
        infra::UniquePtr<TestObject> object = allocator.Allocate();
    }

    infra::UniquePtr<TestObject> object = allocator.Allocate();

    EXPECT_NE(nullptr, object);
}

TEST_F(FixedSpaceAllocatorTest, when_allocating_more_than_capacity_nullptr_is_returned)
{
    infra::UniquePtr<TestObject> firstObject = allocator.Allocate();
    infra::UniquePtr<TestObject> secondObject = allocator.Allocate();

    EXPECT_EQ(nullptr, secondObject);
}
