#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "infra/util/LogAndAbort.hpp"
#include <cstdlib>

namespace services
{
    const EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    const EchoErrorPolicyAbort echoErrorPolicyAbort;

    void EchoErrorPolicyAbortOnMessageFormatError::MessageFormatError() const
    {
        LOG_AND_ABORT("Echo message format error");
    }

    void EchoErrorPolicyAbortOnMessageFormatError::ServiceNotFound(uint32_t serviceId) const
    {}

    void EchoErrorPolicyAbortOnMessageFormatError::MethodNotFound(uint32_t serviceId, uint32_t methodId) const
    {}

    void EchoErrorPolicyAbort::ServiceNotFound(uint32_t serviceId) const
    {
        LOG_AND_ABORT("Echo service not found: %u", serviceId);
    }

    void EchoErrorPolicyAbort::MethodNotFound(uint32_t serviceId, uint32_t methodId) const
    {
        LOG_AND_ABORT("Echo method not found: %u %u", serviceId, methodId);
    }
}
