#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "infra/event/EventDispatcher.hpp"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "infra/util/test_helper/MockCallback.hpp"

class EventDispatcherWithWeakPtrTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{};

TEST_F(EventDispatcherWithWeakPtrTest, TestSchedule)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback());

    infra::EventDispatcher::Instance().Schedule([&callback, this]() { callback.callback(); });
    ExecuteAllActions();
}

TEST_F(EventDispatcherWithWeakPtrTest, TestScheduleTwice)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).Times(2);

    infra::EventDispatcher::Instance().Schedule([&callback, this]() { callback.callback(); });
    infra::EventDispatcher::Instance().Schedule([&callback, this]() { callback.callback(); });
    ExecuteAllActions();
}

TEST_F(EventDispatcherWithWeakPtrTest, TestExecuteOneEvent)
{
    infra::MockCallback<void()> callback;

    EXPECT_CALL(callback, callback()).Times(0);
    ExecuteFirstAction();

    infra::EventDispatcher::Instance().Schedule([&callback, this]() { callback.callback(); });
    infra::EventDispatcher::Instance().Schedule([&callback, this]() { callback.callback(); });

    EXPECT_CALL(callback, callback()).Times(1);
    ExecuteFirstAction();

    EXPECT_CALL(callback, callback()).Times(1);
    ExecuteFirstAction();
}

TEST_F(EventDispatcherWithWeakPtrTest, TestScheduleSharedPtr)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback());

    infra::SharedObjectAllocatorFixedSize<int, void()>::WithStorage<2> allocator;
    infra::SharedPtr<int> object = allocator.Allocate();
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this](const infra::SharedPtr<int>& object) { callback.callback(); }, object);
    ExecuteAllActions();
}

TEST_F(EventDispatcherWithWeakPtrTest, TestScheduleDestructedSharedPtr)
{
    testing::StrictMock<infra::MockCallback<void()>> callback;

    infra::SharedObjectAllocatorFixedSize<int, void()>::WithStorage<2> allocator;
    infra::SharedPtr<int> object = allocator.Allocate();
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this](const infra::SharedPtr<int>& object) { callback.callback(); }, object);
    object = nullptr;
    ExecuteAllActions();
}
