#ifndef PROTOBUF_ECHO_ERROR_POLICY_HPP
#define PROTOBUF_ECHO_ERROR_POLICY_HPP

#include <cstdint>

namespace services
{
    class EchoErrorPolicy
    {
    protected:
        virtual ~EchoErrorPolicy() = default;

    public:
        virtual void MessageFormatError() const = 0;
        virtual void ServiceNotFound(uint32_t serviceId) const = 0;
        virtual void MethodNotFound(uint32_t serviceId, uint32_t methodId) const = 0;
    };

    class EchoErrorPolicyAbortOnMessageFormatError
        : public EchoErrorPolicy
    {
    public:
        void MessageFormatError() const override;
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

    extern const EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    extern const EchoErrorPolicyAbort echoErrorPolicyAbort;
}

#endif
