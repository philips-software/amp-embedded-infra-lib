#ifndef INFRA_STREAM_ERROR_POLICY_HPP
#define INFRA_STREAM_ERROR_POLICY_HPP

#include <cstdlib>

namespace infra
{
    extern const struct SoftFail
    {
    } softFail;

    extern const struct NoFail
    {
    } noFail;

    class StreamErrorPolicy
    {
    public:
        StreamErrorPolicy() = default;
        explicit StreamErrorPolicy(SoftFail);
        explicit StreamErrorPolicy(NoFail);
        StreamErrorPolicy(const StreamErrorPolicy& other) = default;
        StreamErrorPolicy& operator=(const StreamErrorPolicy& other) = delete;
        ~StreamErrorPolicy();

        virtual bool Failed() const;
        virtual void ReportResult(bool ok);

    private:
        enum class FailureMode
        {
            assertion,
            soft,
            no
        };

        FailureMode failureMode = FailureMode::assertion;
        bool failed = false;
        mutable bool checkedFail = true;
    };
}

#endif
