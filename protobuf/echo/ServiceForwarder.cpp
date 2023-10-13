#include "protobuf/echo/ServiceForwarder.hpp"

namespace services
{
    ServiceForwarderBase::ServiceForwarderBase(Echo& echo, Echo& forwardTo)
        : Service(echo)
        , ServiceProxy(forwardTo, 0)
    {}

    infra::SharedPtr<MethodDeserializer> ServiceForwarderBase::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, EchoErrorPolicy& errorPolicy)
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

    void ServiceForwarderBase::MethodContents(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
    {
        this->reader = reader;
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

    bool ServiceForwarderBase::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        this->writer = writer;
        auto result = processedSize + writer->Available() < forwardingSize;
        Transfer();

        return result;
    }

    void ServiceForwarderBase::Transfer()
    {
        if (!sentHeader)
        {
            if (writer == nullptr)
                return;

            infra::DataOutputStream::WithErrorPolicy stream(*writer);

            infra::ProtoFormatter formatter(stream);
            formatter.PutVarInt(forwardingServiceId);
            formatter.PutLengthDelimitedSize(forwardingSize, forwardingMethodId);

            sentHeader = true;
        }

        infra::StreamErrorPolicy errorPolicy;

        while (processedSize != forwardingSize && writer != nullptr && reader != nullptr)
        {
            auto range = reader->ExtractContiguousRange(forwardingSize - processedSize);
            writer->Insert(range, errorPolicy);
            processedSize += range.size();
        }

        if (processedSize == forwardingSize || (reader != nullptr && reader->Empty()))
            reader = nullptr;
        if (processedSize == forwardingSize || (writer != nullptr && writer->Empty()))
            writer = nullptr;
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
