#include "protobuf/echo/test_doubles/ServiceStub.hpp"
#include <cstdint>

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

    const uint32_t& Message::GetDecayed(std::integral_constant<uint32_t, 0>) const
    {
        return value;
    }

    ServiceStub::ServiceStub(Echo& echo, uint32_t serviceId)
        : Service(echo, serviceId)
    {}

    infra::SharedPtr<MethodDeserializer> ServiceStub::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const services::EchoErrorPolicy& errorPolicy)
    {
        switch (methodId)
        {
            case idMethod:
                return Rpc().SerializerFactory().MakeDeserializer<Message, uint32_t>(infra::Function<void(uint32_t)>([this](uint32_t v)
                    {
                        Method(v);
                    }));
            case idMethodNoParameter:
                return Rpc().SerializerFactory().MakeDeserializer<EmptyMessage>(infra::Function<void()>([this]()
                    {
                        MethodNoParameter();
                    }));
            default:
                errorPolicy.MethodNotFound(serviceId, methodId);
                return Rpc().SerializerFactory().MakeDummyDeserializer(Rpc());
        }
    }

    ServiceStubProxy::ServiceStubProxy(services::Echo& echo, uint32_t serviceId)
        : services::ServiceProxy(echo, maxMessageSize)
        , serviceId(serviceId)
    {}

    void ServiceStubProxy::Method(uint32_t value)
    {
        auto serializer = Rpc().SerializerFactory().MakeSerializer<Message, uint32_t>(serviceId, idMethod, value);
        SetSerializer(serializer);
    }

    void ServiceStubProxy::MethodNoParameter()
    {
        auto serializer = Rpc().SerializerFactory().MakeSerializer<EmptyMessage>(serviceId, idMethodNoParameter);
        SetSerializer(serializer);
    }
}
