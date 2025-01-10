#include "services/echo_console/ConsoleService.hpp"

namespace services
{
    void GenericMethodDeserializer::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {}

    void GenericMethodDeserializer::ExecuteMethod()
    {}

    bool GenericMethodDeserializer::Failed() const
    {
        return false;
    }

    ConsoleServiceMethodExecute::ConsoleServiceMethodExecute(application::Console& console)
        : console(console)
    {}

    infra::SharedPtr<MethodDeserializer> ConsoleServiceMethodExecute::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy)
    {
        return methodDeserializer.Emplace();
    }

    ConsoleService::ConsoleService(Echo& echo, uint32_t serviceId, ConsoleServiceMethodExecute& methodExecute)
        : Service(echo, serviceId)
        , methodExecute(methodExecute)
    {}

    infra::SharedPtr<MethodDeserializer> ConsoleService::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy)
    {
        return methodExecute.StartMethod(serviceId, methodId, size, errorPolicy);
    }

    GenericMethodSerializer::GenericMethodSerializer(infra::SharedPtr<infra::StringInputStream>& inputStream)
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

    void ConsoleServiceProxy::SendMessage(infra::SharedPtr<infra::StringInputStream>& inputStream)
    {
        auto serializer = methodSerializer.Emplace(inputStream);
        SetSerializer(serializer);
    }
}
