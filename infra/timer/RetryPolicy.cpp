#include "infra/timer/RetryPolicy.hpp"

namespace infra
{
    RetryPolicyExponentialBackoff::RetryPolicyExponentialBackoff()
        : RetryPolicyExponentialBackoff(std::chrono::hours(24))
    {}

    RetryPolicyExponentialBackoff::RetryPolicyExponentialBackoff(infra::Duration maximumDelay)
        : maximumDelay(maximumDelay)
    {}

    infra::Duration RetryPolicyExponentialBackoff::RetryDelay(bool intermittentFailure)
    {
        if (retryDelay)
        {
            if (infra::Now() >= retryAfterMaximumDelay)
            {
                retryAfterMaximumDelay += maximumDelay;
                *retryDelay = InitialDelay(intermittentFailure);
            }
            else
            {
                *retryDelay *= 2;

                if (infra::Now() + *retryDelay >= retryAfterMaximumDelay)
                    *retryDelay = retryAfterMaximumDelay - infra::Now();
                else if (!intermittentFailure && InitialDelay(false) > *retryDelay)
                    *retryDelay = InitialDelay(false);
            }
        }
        else
        {
            retryAfterMaximumDelay = infra::Now() + maximumDelay;
            retryDelay = InitialDelay(intermittentFailure);
        }

        return *retryDelay;
    }

    void RetryPolicyExponentialBackoff::Reset()
    {
        retryDelay = infra::none;
    }

    infra::Duration RetryPolicyExponentialBackoff::InitialDelay(bool intermittentFailure) const
    {
        if (intermittentFailure)
            return std::chrono::minutes(1);
        else
            return std::chrono::hours(1);
    }

    RetryPolicyFixedInterval::RetryPolicyFixedInterval(infra::Duration delay)
        : retryDelay(delay)
    {}

    infra::Duration RetryPolicyFixedInterval::RetryDelay(bool intermittentFailure)
    {
        return retryDelay;
    }
}
