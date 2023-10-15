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
        ServiceForwarderBase(Echo& echo, Echo& forwardTo);

        // Implementation of Service
        infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, EchoErrorPolicy& errorPolicy) override;

    private:
        // Implementation of MethodDeserializer
        void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

        // Implementation of MethodSerializer
        bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        void Transfer();

    private:
        uint32_t forwardingServiceId;
        uint32_t forwardingMethodId;
        uint32_t forwardingSize;
        uint32_t processedSize;
        bool sentHeader;

        infra::SharedPtr<infra::StreamReaderWithRewinding> contentsReader;
        infra::SharedPtr<infra::StreamWriter> contentsWriter;
    };

    class ServiceForwarderAll
        : public ServiceForwarderBase
    {
    public:
        using ServiceForwarderBase::ServiceForwarderBase;

        bool AcceptsService(uint32_t id) const override;
    };

    class ServiceForwarder
        : public ServiceForwarderBase
    {
    public:
        ServiceForwarder(Echo& echo, uint32_t id, Echo& forwardTo);

        bool AcceptsService(uint32_t id) const override;

    private:
        uint32_t serviceId;
    };
}

#endif
