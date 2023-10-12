#ifndef PROTOBUF_ECHO_SERVICE_FORWARDER_HPP
#define PROTOBUF_ECHO_SERVICE_FORWARDER_HPP

#include "protobuf/echo/Echo.hpp"

namespace services
{
    class ServiceForwarderBase
        : public Service
        , private ServiceProxy
        , private MethodDeserializer
        , private MethodSerializer
    {
    public:
        ServiceForwarderBase(infra::ByteRange messageBuffer, Echo& echo, Echo& forwardTo);

        // Implementation of Service
        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, EchoErrorPolicy& errorPolicy) override;

    private:
        // Implementation of MethodDeserializer
        void MethodContents(infra::StreamReaderWithRewinding& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

        // Implementation of MethodSerializer
        bool SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        const infra::ByteRange messageBuffer;
        infra::Optional<infra::ByteRange> bytes;
        uint32_t forwardingServiceId;
        uint32_t forwardingMethodId;
        uint32_t processedSize;
    };

    class ServiceForwarderAll
        : public ServiceForwarderBase
    {
    public:
        using ServiceForwarderBase::ServiceForwarderBase;

        template<std::size_t MaxMessageSize>
        using WithMaxMessageSize = infra::WithStorage<ServiceForwarderAll, std::array<uint8_t, MaxMessageSize>>;

        bool AcceptsService(uint32_t id) const override;
    };

    class ServiceForwarder
        : public ServiceForwarderBase
    {
    public:
        ServiceForwarder(infra::ByteRange messageBuffer, Echo& echo, uint32_t id, Echo& forwardTo);

        template<std::size_t MaxMessageSize>
        using WithMaxMessageSize = infra::WithStorage<ServiceForwarder, std::array<uint8_t, MaxMessageSize>>;

        bool AcceptsService(uint32_t id) const override;

    private:
        uint32_t serviceId;
    };
}

#endif
