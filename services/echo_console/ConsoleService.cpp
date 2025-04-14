#include "services/echo_console/ConsoleService.hpp"
#include "infra/stream/ByteInputStream.hpp"

namespace services
{
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
