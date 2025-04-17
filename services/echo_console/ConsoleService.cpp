#include "services/echo_console/ConsoleService.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/stream/InputStream.hpp"
#include "infra/stream/StdVectorOutputStream.hpp"
#include "infra/util/SharedPtr.hpp"
#include "protobuf/echo/Echo.hpp"
#include "protobuf/echo/Serialization.hpp"
#include <cstdint>

namespace services
{
    EchoConsoleMethodDeserializer::EchoConsoleMethodDeserializer(uint32_t serviceId, uint32_t methodId, uint32_t size, EchoConsoleMethodExecution& methodExecution)
        : serviceId(serviceId)
        , methodId(methodId)
        , size(size)
        , methodExecution(methodExecution)
    {}

    void EchoConsoleMethodDeserializer::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        infra::StdVectorOutputStream output(stream.Storage());
        if (!sentHeader)
        {
            infra::ProtoFormatter formatter(output);
            formatter.PutVarInt(serviceId);
            formatter.PutLengthDelimitedSize(size, methodId);

            sentHeader = true;
        }

        infra::StreamErrorPolicy errorPolicy;

        while (!reader->Empty())
        {
            auto range = reader->ExtractContiguousRange(reader->Available());
            output << range;
        }
    }

    void EchoConsoleMethodDeserializer::ExecuteMethod()
    {
        methodExecution.ExecuteMethod(stream.Reader());
    }

    bool EchoConsoleMethodDeserializer::Failed() const
    {
        return false;
    }

    EchoConsoleService::EchoConsoleService(Echo& echo, uint32_t acceptExcept, EchoConsoleMethodExecution& methodExecution)
        : Service(echo, ServiceId::reservedServiceId)
        , acceptExcept(acceptExcept)
        , methodExecution(methodExecution)
    {}

    infra::SharedPtr<MethodDeserializer> EchoConsoleService::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy)
    {
        return methodDeserializer.Emplace(serviceId, methodId, size, *this);
    }

    bool EchoConsoleService::AcceptsService(uint32_t id) const
    {
        return id != acceptExcept;
    }

    void EchoConsoleService::ExecuteMethod(infra::StreamReader& data)
    {
        methodExecution.ExecuteMethod(data);
        MethodDone();
    }

    EchoConsoleMethodSerializer::EchoConsoleMethodSerializer(infra::SharedPtr<infra::ByteInputStream>& inputStream)
        : inputStream(inputStream)
    {}

    bool EchoConsoleMethodSerializer::Serialize(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer, infra::softFail);

        stream << inputStream->ContiguousRange(stream.Available());

        auto partlySent = stream.Failed() || inputStream->Available();
        writer = nullptr;

        return partlySent;
    }

    EchoConsoleServiceProxy::EchoConsoleServiceProxy(Echo& echo, uint32_t maxMessageSize)
        : ServiceProxy(echo, maxMessageSize)
    {}

    void EchoConsoleServiceProxy::SendMessage(infra::SharedPtr<infra::ByteInputStream>& inputStream)
    {
        auto serializer = methodSerializer.Emplace(inputStream);
        SetSerializer(serializer);
    }
}
