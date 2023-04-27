#ifndef PROTOBUF_ECHO_SERVICE_FORWARDER_HPP
#define PROTOBUF_ECHO_SERVICE_FORWARDER_HPP

#include "protobuf/echo/Echo.hpp"

namespace services
{
    class ServiceForwarderBase
        : public services::Service
        , private services::ServiceProxy
    {
    public:
        ServiceForwarderBase(infra::ByteRange messageBuffer, Echo& echo, Echo& forwardTo);

        virtual void Handle(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy) override;

    private:
        const infra::ByteRange messageBuffer;
        infra::Optional<infra::ByteRange> bytes;
        uint32_t forwardingServiceId;
    };

    class ServiceForwarderAll
        : public ServiceForwarderBase
    {
    public:
        using ServiceForwarderBase::ServiceForwarderBase;

        template<std::size_t MaxMessageSize>
        using WithMaxMessageSize = infra::WithStorage<ServiceForwarderAll, std::array<uint8_t, MaxMessageSize>>;

        virtual bool AcceptsService(uint32_t id) const override;
    };

    class ServiceForwarder
        : public ServiceForwarderBase
    {
    public:
        ServiceForwarder(infra::ByteRange messageBuffer, Echo& echo, uint32_t id, Echo& forwardTo);

        template<std::size_t MaxMessageSize>
        using WithMaxMessageSize = infra::WithStorage<ServiceForwarder, std::array<uint8_t, MaxMessageSize>>;

        virtual bool AcceptsService(uint32_t id) const override;

    private:
        uint32_t serviceId;
    };
}

#endif
