#include "infra/event/AtomicTriggerScheduler.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include <gtest/gtest.h>

class AtomicTriggerSchedulerTest
    : public infra::EventDispatcherFixture
    , public testing::Test
{};

TEST_F(AtomicTriggerSchedulerTest, scheduled_once)
{
    infra::VerifyingFunction<void()> mock;
    infra::AtomicTriggerScheduler scheduler;

    scheduler.Schedule([&]()
        {
            mock.callback();
        });
    scheduler.Schedule([&]()
        {
            mock.callback();
        });

    ExecuteAllActions();
}
