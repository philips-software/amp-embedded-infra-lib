#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/event/test_helper/EventDispatcherWithWeakPtrFixture.hpp"
#include "infra/util/SharedObjectAllocatorFixedSize.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class EventDispatcherWithWeakPtrTest
    : public testing::Test
    , public infra::EventDispatcherWithWeakPtrFixture
{};

TEST_F(EventDispatcherWithWeakPtrTest, one_action_is_executed)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback());

    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });
    ExecuteAllActions();
}

TEST_F(EventDispatcherWithWeakPtrTest, two_actions_are_executed)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).Times(2);

    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });
    ExecuteAllActions();
}

TEST_F(EventDispatcherWithWeakPtrTest, ExecuteFirstAction_executes_first_action)
{
    infra::MockCallback<void()> callback;

    EXPECT_CALL(callback, callback()).Times(0);
    ExecuteFirstAction();

    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });

    EXPECT_CALL(callback, callback()).Times(1);
    ExecuteFirstAction();

    EXPECT_CALL(callback, callback()).Times(1);
    ExecuteFirstAction();
}

TEST_F(EventDispatcherWithWeakPtrTest, scheduled_action_with_SharedPtr_is_executed)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback());

    infra::SharedObjectAllocatorFixedSize<int, void()>::WithStorage<2> allocator;
    infra::SharedPtr<int> object = allocator.Allocate();
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this](const infra::SharedPtr<int>& object)
        {
            callback.callback();
        },
        object);
    ExecuteAllActions();
}

TEST_F(EventDispatcherWithWeakPtrTest, scheduled_action_with_destructed_SharedPtr_is_not_executed)
{
    testing::StrictMock<infra::MockCallback<void()>> callback;

    infra::SharedObjectAllocatorFixedSize<int, void()>::WithStorage<2> allocator;
    infra::SharedPtr<int> object = allocator.Allocate();
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&callback, this](const infra::SharedPtr<int>& object)
        {
            callback.callback();
        },
        object);
    object = nullptr;
    ExecuteAllActions();
}

class EventDispatcherWithWeakPtrMock
    : public infra::EventDispatcherWithWeakPtr::WithSize<50>
{
public:
    MOCK_METHOD(void, RequestExecution, (), (override));
    MOCK_METHOD(void, Idle, (), (override));
};

class EventDispatcherWithWeakPtrDetailedTest
    : public testing::Test
    , public EventDispatcherWithWeakPtrMock
{};

TEST_F(EventDispatcherWithWeakPtrDetailedTest, ExecuteUntil_executes_until_first_action_sets_predicate_true)
{
    bool done = false;

    EXPECT_CALL(*this, RequestExecution());
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&]()
        {
            done = true;
        });

    ExecuteUntil([&]()
        {
            return done == true;
        });
}

TEST_F(EventDispatcherWithWeakPtrDetailedTest, ExecuteUntil_executes_until_idle_sets_predicate_true)
{
    bool done = false;

    EXPECT_CALL(*this, RequestExecution());
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&]() {});

    EXPECT_CALL(*this, Idle()).WillOnce(testing::Invoke([&]()
        {
            done = true;
        }));

    ExecuteUntil([&]()
        {
            return done == true;
        });
}

TEST_F(EventDispatcherWithWeakPtrDetailedTest, ExecuteUntil_executes_until_second_action_sets_predicate_true)
{
    bool done = false;

    EXPECT_CALL(*this, RequestExecution());
    infra::EventDispatcherWithWeakPtr::Instance().Schedule([&]() {});

    EXPECT_CALL(*this, Idle()).WillOnce(testing::Invoke([&]()
        {
            EXPECT_CALL(*this, RequestExecution());
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([&]()
                {
                    done = true;
                });
        }));

    ExecuteUntil([&]()
        {
            return done == true;
        });
}
