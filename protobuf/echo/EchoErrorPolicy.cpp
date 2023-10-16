#include "protobuf/echo/EchoErrorPolicy.hpp"

namespace services
{
    EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    EchoErrorPolicyAbort echoErrorPolicyAbort;

    void EchoErrorPolicyAbortOnMessageFormatError::MessageFormatError()
    {
        std::abort();
    }

    void EchoErrorPolicyAbortOnMessageFormatError::ServiceNotFound(uint32_t serviceId)
    {}

    void EchoErrorPolicyAbortOnMessageFormatError::MethodNotFound(uint32_t serviceId, uint32_t methodId)
    {}

    void EchoErrorPolicyAbort::ServiceNotFound(uint32_t serviceId)
    {
        std::abort();
    }

    void EchoErrorPolicyAbort::MethodNotFound(uint32_t serviceId, uint32_t methodId)
    {
        std::abort();
    }
}
