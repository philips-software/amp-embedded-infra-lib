#ifndef INFRA_RETRY_HPP
#define INFRA_RETRY_HPP

#include "infra/timer/RetryPolicy.hpp"
#include "infra/timer/Timer.hpp"

namespace infra
{
    enum class FailureType : uint8_t
    {
        intermittentFailure,
        permanentFailure
    };

    template<class T>
    class Retry
    {
    public:
        static_assert(std::is_base_of<infra::RetryPolicy, T>::value, "T should derive from infra::RetryPolicy");

        Retry() = default;

        template<class... Args>
        Retry(Args&&... args)
            : policy(std::forward<Args>(args)...)
        {}

        void Start(FailureType type, const infra::Function<void()>& action);
        void Stop();

    private:
        T policy;
        infra::TimerSingleShot timer;
    };

    template<class T>
    void Retry<T>::Start(FailureType type, const infra::Function<void()>& action)
    {
        switch (type)
        {
            case FailureType::intermittentFailure:
                timer.Start(policy.RetryDelay(true), action);
                break;
            case FailureType::permanentFailure:
                timer.Start(policy.RetryDelay(false), action);
                break;
        }
    }

    template<class T>
    void Retry<T>::Stop()
    {
        timer.Cancel();
        policy.Reset();
    }

    using RetryFixedInterval = Retry<RetryPolicyFixedInterval>;
    using RetryExponentialBackoff = Retry<RetryPolicyExponentialBackoff>;
}

#endif
