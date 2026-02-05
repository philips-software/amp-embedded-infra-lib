#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "infra/util/LogAndAbort.hpp"
#include <cstdint>
#include <cstdlib>

namespace services
{
    void EchoErrorPolicyAbortOnMessageFormatError::MessageFormatError(const char* reason) const
    {
        LOG_AND_ABORT("Echo message format error: %s", reason);
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
