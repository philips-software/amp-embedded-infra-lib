#include "infra/timer/Waiting.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gtest/gtest.h"

class WaitingTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(WaitingTest, WaitUntilDone_fails_when_nothing_happens)
{
    infra::EventDispatcher::Instance().Schedule([this]()
        {
            systemTimerService.TimeProgressed(std::chrono::seconds(1));
        });

    EXPECT_FALSE(infra::WaitUntilDone([](const infra::Function<void()>& onDone) {}));
};

TEST_F(WaitingTest, WaitUntilDone_is_successful_when_onDone_is_invoked)
{
    EXPECT_TRUE(infra::WaitUntilDone([](const infra::Function<void()>& onDone)
        {
            onDone();
        }));
};

TEST_F(WaitingTest, WaitFor_fails_when_predicate_is_false)
{
    infra::EventDispatcher::Instance().Schedule([this]()
        {
            systemTimerService.TimeProgressed(std::chrono::seconds(1));
        });

    EXPECT_FALSE(infra::WaitFor([]()
        {
            return false;
        }));
};

TEST_F(WaitingTest, WaitFor_is_successful_when_predicate_is_true)
{
    EXPECT_TRUE(infra::WaitFor([]()
        {
            return true;
        }));
};

TEST_F(WaitingTest, host_WaitUntilDone_fails_when_nothing_happens)
{
    infra::EventDispatcher::Instance().Schedule([this]()
        {
            systemTimerService.TimeProgressed(std::chrono::seconds(1));
        });

    EXPECT_FALSE(infra::host::WaitUntilDone([](const std::function<void()>& onDone) {}));
};

TEST_F(WaitingTest, host_WaitUntilDone_is_successful_when_onDone_is_invoked)
{
    EXPECT_TRUE(infra::host::WaitUntilDone([](const std::function<void()>& onDone)
        {
            onDone();
        }));
};

TEST_F(WaitingTest, host_WaitFor_fails_when_predicate_is_false)
{
    infra::EventDispatcher::Instance().Schedule([this]()
        {
            systemTimerService.TimeProgressed(std::chrono::seconds(1));
        });

    EXPECT_FALSE(infra::host::WaitFor([]()
        {
            return false;
        }));
};

TEST_F(WaitingTest, host_WaitFor_is_successful_when_predicate_is_true)
{
    EXPECT_TRUE(infra::host::WaitFor([]()
        {
            return true;
        }));
}
