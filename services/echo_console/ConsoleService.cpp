#include "services/echo_console/ConsoleService.hpp"
#include "infra/stream/ByteInputStream.hpp"

namespace services
{
    ConsoleService::ConsoleService(Echo& echo, uint32_t serviceId, GenericMethodDeserializerFactory& deserializerFactory)
        : Service(echo, serviceId)
        , methodDeserializerFactory(deserializerFactory)
    {}

    infra::SharedPtr<MethodDeserializer> ConsoleService::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy)
    {
        return methodDeserializerFactory.MakeDeserializer(serviceId, methodId, size);
    }

    GenericMethodSerializer::GenericMethodSerializer(infra::SharedPtr<infra::ByteInputStream>& inputStream)
        : inputStream(inputStream)
    {}

    bool GenericMethodSerializer::Serialize(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer, infra::softFail);

        stream << inputStream->ContiguousRange(stream.Available());

        auto partlySent = stream.Failed() || inputStream->Available();
        writer = nullptr;

        return partlySent;
    }

    ConsoleServiceProxy::ConsoleServiceProxy(Echo& echo, uint32_t maxMessageSize)
        : ServiceProxy(echo, maxMessageSize)
    {}

    void ConsoleServiceProxy::SendMessage(infra::SharedPtr<infra::ByteInputStream>& inputStream)
    {
        auto serializer = methodSerializer.Emplace(inputStream);
        SetSerializer(serializer);
    }
}
