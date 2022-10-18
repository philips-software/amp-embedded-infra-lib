#include "infra/timer/RetryPolicy.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "gtest/gtest.h"
#include <cmath>

class RetryPolicyTest
    : public testing::Test
    , public infra::ClockFixture
{};

TEST_F(RetryPolicyTest, should_return_retry_delay_after_intermittent_failure)
{
    infra::RetryPolicyExponentialBackoff policy;
    ASSERT_EQ(std::chrono::minutes(1), policy.RetryDelay(true));
}

TEST_F(RetryPolicyTest, should_return_retry_delay_after_non_intermittent_failure)
{
    infra::RetryPolicyExponentialBackoff policy;
    ASSERT_EQ(std::chrono::hours(1), policy.RetryDelay(false));
}

TEST_F(RetryPolicyTest, should_return_exponentially_larger_delay)
{
    infra::RetryPolicyExponentialBackoff policy;
    infra::Duration initialDelay = std::chrono::minutes(1);

    for (int i = 0; i < 10; ++i)
        ASSERT_EQ(std::pow(2, i) * initialDelay, policy.RetryDelay(true));
}

TEST_F(RetryPolicyTest, should_return_retry_delay_after_intermittent_failure_followed_by_non_intermittent_failure)
{
    infra::RetryPolicyExponentialBackoff policy;

    policy.RetryDelay(true);

    EXPECT_EQ(std::chrono::hours(1), policy.RetryDelay(false));
}

TEST_F(RetryPolicyTest, should_reset_retry_delay)
{
    infra::RetryPolicyExponentialBackoff policy;

    policy.RetryDelay(true);
    policy.RetryDelay(true);
    policy.Reset();

    ASSERT_EQ(std::chrono::minutes(1), policy.RetryDelay(true));
}

TEST_F(RetryPolicyTest, should_return_maximum_retry_delay_when_maximum_is_reached)
{
    infra::RetryPolicyExponentialBackoff policy(std::chrono::minutes(4));

    ASSERT_EQ(std::chrono::minutes(1), policy.RetryDelay(true));
    ASSERT_EQ(std::chrono::minutes(2), policy.RetryDelay(true));
    ASSERT_EQ(std::chrono::minutes(4), policy.RetryDelay(true));
    ASSERT_EQ(std::chrono::minutes(4), policy.RetryDelay(true));
}

TEST_F(RetryPolicyTest, should_return_initial_retry_delay_after_waiting_for_maximum_delay_time)
{
    infra::RetryPolicyExponentialBackoff policy(std::chrono::minutes(4));

    for (int i = 0; i < 3; ++i)
        policy.RetryDelay(true);

    ForwardTime(std::chrono::minutes(4));

    ASSERT_EQ(std::chrono::minutes(1), policy.RetryDelay(true));
}
