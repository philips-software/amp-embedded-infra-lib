#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "infra/stream/StdStringOutputStream.hpp"
#include "infra/util/ReallyAssert.hpp"
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
        infra::StdStringOutputStream::WithStorage stream;
        stream << "EchoErrorPolicyAbort::ServiceNotFound - serviceId: " << serviceId;
        really_assert(false);
        std::abort();
    }

    void EchoErrorPolicyAbort::MethodNotFound(uint32_t serviceId, uint32_t methodId) const
    {
        infra::StdStringOutputStream::WithStorage stream;
        stream << "EchoErrorPolicyAbort::MethodNotFound - serviceId: " << serviceId << ", methodId: " << methodId;
        really_assert(false);
        std::abort();
    }
}
