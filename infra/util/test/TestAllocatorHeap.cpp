#include "gtest/gtest.h"
#include "infra/util/AllocatorHeap.hpp"

class HeapAllocatorTest
    : public testing::Test
{
public:
    struct TestObject
    {
        void* operator new(size_t size, std::nothrow_t) noexcept
        {
            if (allocated)
                return nullptr;

            allocated = true;

            return malloc(size);
        }

        void operator delete(void* object)
        {
            allocated = false;

            free(object);
        }

        static bool allocated;
    };

    infra::AllocatorHeap<TestObject, void()> allocator;
};

bool HeapAllocatorTest::TestObject::allocated = false;

TEST_F(HeapAllocatorTest, allocate_one_object)
{
    infra::UniquePtr<TestObject> object = allocator.Allocate();

    EXPECT_TRUE(TestObject::allocated);
}

TEST_F(HeapAllocatorTest, after_object_goes_out_of_scope_object_is_released)
{
    {
        infra::UniquePtr<TestObject> object = allocator.Allocate();
    }

    EXPECT_FALSE(TestObject::allocated);
}

TEST_F(HeapAllocatorTest, when_allocating_more_than_capacity_nullptr_is_returned)
{
    infra::UniquePtr<TestObject> firstObject = allocator.Allocate();
    infra::UniquePtr<TestObject> secondObject = allocator.Allocate();

    EXPECT_EQ(nullptr, secondObject);
}
