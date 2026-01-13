#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "services/tracer/GlobalTracer.hpp"
#include <cstdlib>

namespace services
{
    const EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    const EchoErrorPolicyAbort echoErrorPolicyAbort;

    void EchoErrorPolicyAbortOnMessageFormatError::MessageFormatError() const
    {
        std::abort();
    }

    void EchoErrorPolicyAbortOnMessageFormatError::ServiceNotFound(uint32_t serviceId) const
    {}

    void EchoErrorPolicyAbortOnMessageFormatError::MethodNotFound(uint32_t serviceId, uint32_t methodId) const
    {}

    void EchoErrorPolicyAbort::ServiceNotFound(uint32_t serviceId) const
    {
        services::GlobalTracer().Trace() << "EchoErrorPolicyAbort::ServiceNotFound - serviceId: " << serviceId;
        std::abort();
    }

    void EchoErrorPolicyAbort::MethodNotFound(uint32_t serviceId, uint32_t methodId) const
    {
        services::GlobalTracer().Trace() << "EchoErrorPolicyAbort::MethodNotFound - serviceId: " << serviceId << ", methodId: " << methodId;
        std::abort();
    }
}
