#ifndef PROTOBUF_ECHO_ERROR_POLICY_HPP
#define PROTOBUF_ECHO_ERROR_POLICY_HPP

#include "infra/stream/BufferingStreamReader.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Compatibility.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/Optional.hpp"
#include "protobuf/echo/Proto.hpp"
#include "protobuf/echo/Serialization.hpp"

namespace services
{
    class EchoErrorPolicy
    {
    protected:
        ~EchoErrorPolicy() = default;

    public:
        virtual void MessageFormatError() = 0;
        virtual void ServiceNotFound(uint32_t serviceId) = 0;
        virtual void MethodNotFound(uint32_t serviceId, uint32_t methodId) = 0;
    };

    class EchoErrorPolicyAbortOnMessageFormatError
        : public EchoErrorPolicy
    {
    public:
        void MessageFormatError() override;
        void ServiceNotFound(uint32_t serviceId) override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) override;
    };

    class EchoErrorPolicyAbort
        : public EchoErrorPolicyAbortOnMessageFormatError
    {
    public:
        void ServiceNotFound(uint32_t serviceId) override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) override;
    };

    extern EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    extern EchoErrorPolicyAbort echoErrorPolicyAbort;
}

#endif
