#include "gmock/gmock.h"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "infra/util/SharedPtr.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MonitoredConstructionObject.hpp"

using AllocatorMySharedObject = infra::SharedObjectAllocator<infra::MonitoredConstructionObject, void(infra::ConstructionMonitorMock&)>;

class SharedObjectAllocatorFixedSizeTest
    : public testing::Test
{
public:
    testing::StrictMock<infra::ConstructionMonitorMock> objectConstructionMock;
};

TEST_F(SharedObjectAllocatorFixedSizeTest, allocate_one_object)
{
    AllocatorMySharedObject::UsingAllocator<infra::SharedObjectAllocatorFixedSize>::WithStorage<2> allocator;

    void* savedObject;
    EXPECT_CALL(objectConstructionMock, Construct(testing::_)).WillOnce(testing::SaveArg<0>(&savedObject));
    infra::SharedPtr<infra::MonitoredConstructionObject> object = allocator.Allocate(objectConstructionMock);
    EXPECT_TRUE(static_cast<bool>(object));
    EXPECT_CALL(objectConstructionMock, Destruct(savedObject));
}

TEST_F(SharedObjectAllocatorFixedSizeTest, when_allocation_fails_empty_SharedPtr_is_returned)
{
    infra::SharedObjectAllocatorFixedSize<infra::MonitoredConstructionObject, void(infra::ConstructionMonitorMock&)>::WithStorage<0> allocator;

    infra::SharedPtr<infra::MonitoredConstructionObject> object = allocator.Allocate(objectConstructionMock);
    EXPECT_FALSE(static_cast<bool>(object));
}

TEST_F(SharedObjectAllocatorFixedSizeTest, object_is_destructed_but_not_deallocated_while_WeakPtr_has_a_reference)
{
    infra::SharedObjectAllocatorFixedSize<infra::MonitoredConstructionObject, void(infra::ConstructionMonitorMock&)>::WithStorage<1> allocator;

    infra::WeakPtr<infra::MonitoredConstructionObject> weakObject;

    {
        EXPECT_CALL(objectConstructionMock, Construct(testing::_));
        infra::SharedPtr<infra::MonitoredConstructionObject> object = allocator.Allocate(objectConstructionMock);
        weakObject = object;
        EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
    }
    testing::Mock::VerifyAndClearExpectations(&objectConstructionMock);

    EXPECT_EQ(nullptr, allocator.Allocate(objectConstructionMock));
}

TEST_F(SharedObjectAllocatorFixedSizeTest, invoke_on_allocatable_when_WeakPtrs_expire)
{
    infra::SharedObjectAllocatorFixedSize<infra::MonitoredConstructionObject, void(infra::ConstructionMonitorMock&)>::WithStorage<1> allocator;

    testing::StrictMock<infra::MockCallback<void()>> callback;
    allocator.OnAllocatable([&]()
        { callback.callback(); });

    {
        infra::WeakPtr<infra::MonitoredConstructionObject> weakObject;

        {
            EXPECT_CALL(objectConstructionMock, Construct(testing::_));
            infra::SharedPtr<infra::MonitoredConstructionObject> object = allocator.Allocate(objectConstructionMock);
            weakObject = object;
            EXPECT_CALL(objectConstructionMock, Destruct(testing::_));
        }

        EXPECT_CALL(callback, callback());
    }

    testing::Mock::VerifyAndClearExpectations(&callback);
}
