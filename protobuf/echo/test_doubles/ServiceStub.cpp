#include "protobuf/echo/test_doubles/ServiceStub.hpp"

namespace services
{
    Message::Message(uint32_t value)
        : value(value)
    {}

    void Message::Serialize(infra::ProtoFormatter& formatter) const
    {
        SerializeField(services::ProtoInt32(), formatter, value, 1);
    }

    uint32_t& Message::Get(std::integral_constant<uint32_t, 0>)
    {
        return value;
    }

    const uint32_t& Message::Get(std::integral_constant<uint32_t, 0>) const
    {
        return value;
    }

    ServiceStub::ServiceStub(Echo& echo, MethodSerializerFactory& serializerFactory)
        : Service(echo)
        , serializerFactory(serializerFactory)
    {}

    bool ServiceStub::AcceptsService(uint32_t id) const
    {
        return id == serviceId;
    }

    infra::SharedPtr<MethodDeserializer> ServiceStub::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, services::EchoErrorPolicy& errorPolicy)
    {
        switch (methodId)
        {
            case idMethod:
                return serializerFactory.MakeDeserializer<Message, uint32_t>(infra::Function<void(uint32_t)>([this](uint32_t v)
                    {
                        Method(v);
                    }));
            default:
                errorPolicy.MethodNotFound(serviceId, methodId);
                return serializerFactory.MakeDummyDeserializer(Rpc());
        }
    }

    ServiceStubProxy::ServiceStubProxy(services::Echo& echo, MethodSerializerFactory& serializerFactory)
        : services::ServiceProxy(echo, maxMessageSize)
        , serializerFactory(serializerFactory)
    {}

    void ServiceStubProxy::Method(uint32_t value)
    {
        auto serializer = serializerFactory.MakeSerializer<Message, uint32_t>(serviceId, idMethod, value);
        SetSerializer(serializer);
    }
}
