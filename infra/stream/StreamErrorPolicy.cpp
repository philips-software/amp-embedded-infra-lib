#include "infra/stream/StreamErrorPolicy.hpp"
#include "infra/util/ReallyAssert.hpp"
#include <cassert>

namespace infra
{
    const SoftFail softFail;
    const NoFail noFail;

    StreamErrorPolicy::StreamErrorPolicy(SoftFail)
        : failureMode(FailureMode::soft)
    {}

    StreamErrorPolicy::StreamErrorPolicy(NoFail)
        : failureMode(FailureMode::no)
    {}

    StreamErrorPolicy::~StreamErrorPolicy()
    {
        assert(checkedFail);
    }

    bool StreamErrorPolicy::Failed() const
    {
        checkedFail = true;
        return failed;
    }

    void StreamErrorPolicy::ReportResult(bool ok)
    {
        if (!ok)
        {
            failed = true;
            really_assert(failureMode != FailureMode::assertion);
        }

        checkedFail = failureMode != FailureMode::soft;
    }
}
