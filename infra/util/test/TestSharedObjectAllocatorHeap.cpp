#include "infra/util/SharedObjectAllocatorHeap.hpp"
#include "infra/util/SharedPtr.hpp"
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
    void* savedObject;
    EXPECT_CALL(objectConstructionMock, Construct(testing::_)).WillOnce(testing::SaveArg<0>(&savedObject));
    infra::SharedPtr<infra::MonitoredConstructionObject> object = allocator.Allocate(objectConstructionMock);
    EXPECT_TRUE(static_cast<bool>(object));
    EXPECT_CALL(objectConstructionMock, Destruct(savedObject));
}
