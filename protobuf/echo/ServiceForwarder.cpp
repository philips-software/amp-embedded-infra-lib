#include "protobuf/echo/ServiceForwarder.hpp"

namespace services
{
    ServiceForwarderBase::ServiceForwarderBase(Echo& echo, Echo& forwardTo)
        : Service(echo)
        , ServiceProxy(forwardTo, 0)
    {}

    infra::SharedPtr<MethodDeserializer> ServiceForwarderBase::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy)
    {
        forwardingServiceId = serviceId;
        forwardingMethodId = methodId;
        forwardingSize = size;
        processedSize = 0;
        sentHeader = false;

        RequestSend([this]()
            {
                SetSerializer(infra::UnOwnedSharedPtr(static_cast<MethodSerializer&>(*this)));
            },
            size);

        return infra::UnOwnedSharedPtr(static_cast<MethodDeserializer&>(*this));
    }

    void ServiceForwarderBase::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        contentsReader = std::move(reader);
        Transfer();
    }

    void ServiceForwarderBase::ExecuteMethod()
    {
        MethodDone();
    }

    bool ServiceForwarderBase::Failed() const
    {
        return false;
    }

    bool ServiceForwarderBase::Serialize(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        contentsWriter = std::move(writer);
        auto result = processedSize + contentsWriter->Available() < forwardingSize;
        Transfer();

        return result;
    }

    void ServiceForwarderBase::Transfer()
    {
        if (!sentHeader)
        {
            if (contentsWriter == nullptr)
                return;

            infra::DataOutputStream::WithErrorPolicy stream(*contentsWriter);

            infra::ProtoFormatter formatter(stream);
            formatter.PutVarInt(forwardingServiceId);
            formatter.PutLengthDelimitedSize(forwardingSize, forwardingMethodId);

            sentHeader = true;
        }

        infra::StreamErrorPolicy errorPolicy;

        while (processedSize != forwardingSize && contentsWriter != nullptr && contentsReader != nullptr)
        {
            auto range = contentsReader->ExtractContiguousRange(forwardingSize - processedSize);
            contentsWriter->Insert(range, errorPolicy);
            processedSize += range.size();
        }

        if (processedSize == forwardingSize || (contentsWriter != nullptr && contentsWriter->Empty()))
            contentsWriter = nullptr;
        if (processedSize == forwardingSize || (contentsReader != nullptr && contentsReader->Empty()))
            contentsReader = nullptr;
    }

    bool ServiceForwarderAll::AcceptsService(uint32_t id) const
    {
        return true;
    }

    ServiceForwarder::ServiceForwarder(Echo& echo, uint32_t id, Echo& forwardTo)
        : ServiceForwarderBase(echo, forwardTo)
        , serviceId(id)
    {}

    bool ServiceForwarder::AcceptsService(uint32_t id) const
    {
        return serviceId == id;
    }
}
