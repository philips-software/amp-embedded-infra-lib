#include "protobuf/echo/test_doubles/ServiceStub.hpp"

namespace services
{
    struct Message
    {
    public:
        static const uint32_t numberOfFields = 1;
        template<std::size_t fieldIndex>
        using ProtoType = ProtoUInt32;
        template<std::size_t fieldIndex>
        using Type = uint32_t;
        template<std::size_t fieldIndex>
        static const uint32_t fieldNumber = 1;

    public:
        uint32_t& Get(std::integral_constant<uint32_t, 0>)
        {
            return value;
        }

        const uint32_t& Get(std::integral_constant<uint32_t, 0>) const
        {
            return value;
        }

    public:
        Message() = default;

        Message(uint32_t value)
            : value(value)
        {}

    public:
        uint32_t value = 0;
    };

    infra::SharedPtr<MethodDeserializer> ServiceStub::StartMethod(uint32_t serviceId, uint32_t methodId, services::EchoErrorPolicy& errorPolicy)
    {
        switch (methodId)
        {
            case idMethod:
                return infra::MakeSharedOnHeap<services::MethodDeserializerImpl<Message, ServiceStub, uint32_t>>(*this, &ServiceStub::Method);
            default:
                errorPolicy.MethodNotFound(serviceId, methodId);
                return infra::MakeSharedOnHeap<services::MethodDeserializerDummy>();
        }
    }

    ServiceStubProxy::ServiceStubProxy(services::Echo& echo)
        : services::ServiceProxy(echo, maxMessageSize)
    {}

    void ServiceStubProxy::Method(uint32_t value)
    {
        auto serializer = infra::MakeSharedOnHeap<MethodSerializerImpl<Message, uint32_t>>(value);
        SetSerializer(serializer);
    }
}
