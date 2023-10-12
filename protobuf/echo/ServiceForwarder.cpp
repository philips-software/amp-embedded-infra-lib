#include "protobuf/echo/ServiceForwarder.hpp"

namespace services
{
    ServiceForwarderBase::ServiceForwarderBase(infra::ByteRange messageBuffer, Echo& echo, Echo& forwardTo)
        : Service(echo)
        , ServiceProxy(forwardTo, messageBuffer.size())
        , messageBuffer(messageBuffer)
    {}

    infra::SharedPtr<MethodDeserializer> ServiceForwarderBase::StartMethod(uint32_t serviceId, uint32_t methodId, EchoErrorPolicy& errorPolicy)
    {
        forwardingServiceId = serviceId;
        forwardingMethodId = methodId;
        processedSize = 0;
        bytes.Emplace(messageBuffer);

        return infra::UnOwnedSharedPtr(static_cast<MethodDeserializer&>(*this));
    }

    void ServiceForwarderBase::MethodContents(infra::StreamReaderWithRewinding& reader)
    {
        while (true)
        {
            infra::ConstByteRange contiguousBytes = reader.ExtractContiguousRange(std::numeric_limits<uint32_t>::max());

            if (contiguousBytes.empty())
                break;

            std::copy(contiguousBytes.begin(), contiguousBytes.end(), bytes->begin() + processedSize);
            processedSize += contiguousBytes.size();
        }
    }

    void ServiceForwarderBase::ExecuteMethod()
    {
        bytes->shrink_from_back_to(processedSize);

        SetSerializer(infra::UnOwnedSharedPtr(static_cast<MethodSerializer&>(*this)));

        uint32_t messageSize = infra::MaxVarIntSize(forwardingServiceId) + infra::MaxVarIntSize((forwardingMethodId << 3) | 2) + infra::MaxVarIntSize(bytes->size()) + bytes->size();

        //todo
        //RequestSend([this]()
        //    {
        //        infra::DataOutputStream::WithErrorPolicy stream(ServiceProxy::Rpc().SendStreamWriter());
        //        infra::ProtoFormatter formatter(stream);
        //        formatter.PutVarInt(forwardingServiceId);
        //        formatter.PutBytesField(*bytes, forwardingMethodId);
        //        bytes = infra::none;

        //        ServiceProxy::Rpc().Send();
        //        MethodDone();
        //    },
        //    messageSize);
    }

    bool ServiceForwarderBase::Failed() const
    {
        return false;
    }

    bool ServiceForwarderBase::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        return false;
        //todo
    }

    bool ServiceForwarderAll::AcceptsService(uint32_t id) const
    {
        return true;
    }

    ServiceForwarder::ServiceForwarder(infra::ByteRange messageBuffer, Echo& echo, uint32_t id, Echo& forwardTo)
        : ServiceForwarderBase(messageBuffer, echo, forwardTo)
        , serviceId(id)
    {}

    bool ServiceForwarder::AcceptsService(uint32_t id) const
    {
        return serviceId == id;
    }
}
