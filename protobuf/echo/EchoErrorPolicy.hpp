#ifndef PROTOBUF_ECHO_ERROR_POLICY_HPP
#define PROTOBUF_ECHO_ERROR_POLICY_HPP

#include "services/tracer/Tracer.hpp"
#include <cstdint>

namespace services
{
    class EchoErrorPolicy
    {
    protected:
        virtual ~EchoErrorPolicy() = default;

    public:
        virtual void MessageFormatError(const char* reason) const = 0;
        virtual void ServiceNotFound(uint32_t serviceId) const = 0;
        virtual void MethodNotFound(uint32_t serviceId, uint32_t methodId) const = 0;
    };

    class EchoErrorPolicyAbortOnMessageFormatError
        : public EchoErrorPolicy
    {
    public:
        void MessageFormatError(const char* reason) const override;
        void ServiceNotFound(uint32_t serviceId) const override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) const override;
    };

    class EchoErrorPolicyAbort
        : public EchoErrorPolicyAbortOnMessageFormatError
    {
    public:
        void ServiceNotFound(uint32_t serviceId) const override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) const override;
    };

    class EchoErrorPolicyWarn
        : public EchoErrorPolicy
    {
    public:
        EchoErrorPolicyWarn(services::Tracer& tracer);
        void MessageFormatError(const char* reason) const override;
        void ServiceNotFound(uint32_t serviceId) const override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) const override;

    private:
        services::Tracer& tracer;
    };

    const inline EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    const inline EchoErrorPolicyAbort echoErrorPolicyAbort;
}

#endif
