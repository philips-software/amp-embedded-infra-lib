#ifndef INFRA_RETRY_POLICY_HPP
#define INFRA_RETRY_POLICY_HPP

#include "infra/timer/Timer.hpp"
#include "infra/util/Optional.hpp"

namespace infra
{
    class RetryPolicy
    {
    public:
        RetryPolicy() = default;
        virtual ~RetryPolicy() = default;
        RetryPolicy(const RetryPolicy&) = delete;
        RetryPolicy& operator=(const RetryPolicy&) = delete;

        virtual infra::Duration RetryDelay(bool intermittentFailure) = 0;
        virtual void Reset() = 0;
    };

    class RetryPolicyExponentialBackoff
        : public RetryPolicy
    {
    public:
        RetryPolicyExponentialBackoff();
        RetryPolicyExponentialBackoff(infra::Duration maximumDelay);

        infra::Duration RetryDelay(bool intermittentFailure) override;
        void Reset() override;

    private:
        infra::Duration InitialDelay(bool intermittentFailure) const;

    private:
        infra::Duration maximumDelay;
        infra::TimePoint retryAfterMaximumDelay;
        infra::Optional<infra::Duration> retryDelay;
    };
}

#endif
