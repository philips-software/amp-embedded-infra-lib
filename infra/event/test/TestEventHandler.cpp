#include "infra/event/EventDispatcher.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class EventDispatcherTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{};

TEST_F(EventDispatcherTest, TestConstruction)
{}

TEST_F(EventDispatcherTest, TestSchedule)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback());

    infra::EventDispatcher::Instance().Schedule([&callback, this]() { callback.callback(); });
    ExecuteAllActions();
}

TEST_F(EventDispatcherTest, TestScheduleTwice)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback()).Times(2);

    infra::EventDispatcher::Instance().Schedule([&callback, this]() { callback.callback(); });
    infra::EventDispatcher::Instance().Schedule([&callback, this]() { callback.callback(); });
    ExecuteAllActions();
}

TEST_F(EventDispatcherTest, TestExecuteOneEvent)
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

TEST_F(EventDispatcherTest, TestPerformance)
{
    for (int j = 0; j != 10; ++j)
    {
        for (int i = 0; i != 20; ++i)
        {
            infra::EventDispatcher::Instance().Schedule([]() { helper1(); });
            infra::EventDispatcher::Instance().Schedule([]() { helper2(); });
        }

        ExecuteAllActions();
    }
}
