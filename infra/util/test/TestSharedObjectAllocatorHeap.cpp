#include "infra/util/SharedObjectAllocatorHeap.hpp"
#include "infra/util/SharedPtr.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "infra/util/test_helper/MonitoredConstructionObject.hpp"
#include <gmock/gmock.h>

using AllocatorMySharedObject = infra::SharedObjectAllocator<infra::MonitoredConstructionObject, void(infra::ConstructionMonitorMock&)>;

class SharedObjectAllocatorHeapTest
    : public testing::Test
{
public:
    testing::StrictMock<infra::ConstructionMonitorMock> objectConstructionMock;
    AllocatorMySharedObject::UsingAllocator<infra::SharedObjectAllocatorHeap> allocator;
};

TEST_F(SharedObjectAllocatorHeapTest, allocate_one_object)
{
    EXPECT_TRUE(allocator.NoneAllocated());

    void* savedObject;
    EXPECT_CALL(objectConstructionMock, Construct(testing::_)).WillOnce(testing::SaveArg<0>(&savedObject));
    infra::SharedPtr<infra::MonitoredConstructionObject> object = allocator.Allocate(objectConstructionMock);
    EXPECT_TRUE(static_cast<bool>(object));

    EXPECT_FALSE(allocator.NoneAllocated());
    EXPECT_CALL(objectConstructionMock, Destruct(savedObject));
    object = nullptr;
    EXPECT_TRUE(allocator.NoneAllocated());
}

TEST_F(SharedObjectAllocatorHeapTest, invoke_on_allocatable_when_WeakPtrs_expire)
{
    testing::StrictMock<infra::MockCallback<void()>> callback;
    allocator.OnAllocatable([&]()
        {
            callback.callback();
        });

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
