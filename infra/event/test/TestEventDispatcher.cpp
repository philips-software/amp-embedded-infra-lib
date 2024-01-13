#include "infra/event/EventDispatcher.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class EventDispatcherTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{};

TEST_F(EventDispatcherTest, scheduled_action_is_executed)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback());

    infra::EventDispatcher::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });
    ExecuteAllActions();
}

TEST_F(EventDispatcherTest, two_actions_are_executed)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).Times(2);

    infra::EventDispatcher::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });
    infra::EventDispatcher::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });
    ExecuteAllActions();
}

TEST_F(EventDispatcherTest, ExecuteFirstAction_executes_first_action)
{
    infra::MockCallback<void()> callback;

    EXPECT_CALL(callback, callback()).Times(0);
    ExecuteFirstAction();

    infra::EventDispatcher::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });
    infra::EventDispatcher::Instance().Schedule([&callback, this]()
        {
            callback.callback();
        });

    EXPECT_CALL(callback, callback()).Times(1);
    ExecuteFirstAction();

    EXPECT_CALL(callback, callback()).Times(1);
    ExecuteFirstAction();
}

class EventDispatcherMock
    : public infra::EventDispatcher::WithSize<50>
{
public:
    MOCK_METHOD(void, RequestExecution, (), (override));
    MOCK_METHOD(void, Idle, (), (override));
};

class EventDispatcherDetailedTest
    : public testing::Test
    , public EventDispatcherMock
{};

TEST_F(EventDispatcherDetailedTest, ExecuteUntil_executes_until_first_action_sets_predicate_true)
{
    bool done = false;

    EXPECT_CALL(*this, RequestExecution());
    infra::EventDispatcher::Instance().Schedule([&]()
        {
            done = true;
        });

    ExecuteUntil([&]()
        {
            return done == true;
        });
}

TEST_F(EventDispatcherDetailedTest, ExecuteUntil_executes_until_idle_sets_predicate_true)
{
    bool done = false;

    EXPECT_CALL(*this, RequestExecution());
    infra::EventDispatcher::Instance().Schedule([&]() {});

    EXPECT_CALL(*this, Idle()).WillOnce(testing::Invoke([&]()
        {
            done = true;
        }));

    ExecuteUntil([&]()
        {
            return done == true;
        });
}

TEST_F(EventDispatcherDetailedTest, ExecuteUntil_executes_until_second_action_sets_predicate_true)
{
    bool done = false;

    EXPECT_CALL(*this, RequestExecution());
    infra::EventDispatcher::Instance().Schedule([&]() {});

    EXPECT_CALL(*this, Idle()).WillOnce(testing::Invoke([&]()
        {
            EXPECT_CALL(*this, RequestExecution());
            infra::EventDispatcher::Instance().Schedule([&]()
                {
                    done = true;
                });
        }));

    ExecuteUntil([&]()
        {
            return done == true;
        });
}

bool helper1turn = true;

void helper1()
{
    really_assert(helper1turn);
    helper1turn = false;
}

void helper2()
{
    really_assert(!helper1turn);
    helper1turn = true;
}

TEST_F(EventDispatcherTest, execute_a_lot)
{
    for (int j = 0; j != 10; ++j)
    {
        for (int i = 0; i != 20; ++i)
        {
            infra::EventDispatcher::Instance().Schedule([]()
                {
                    helper1();
                });
            infra::EventDispatcher::Instance().Schedule([]()
                {
                    helper2();
                });
        }

        ExecuteAllActions();
    }
}
