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
        this->forwardingServiceId = serviceId;
        bytes.Emplace(messageBuffer);
        uint32_t processedSize = 0;

        while (true)
        {
            infra::ConstByteRange contiguousBytes;
            contents.GetBytesReference(contiguousBytes);

            if (contiguousBytes.size() == 0)
                break;

            std::copy(contiguousBytes.begin(), contiguousBytes.end(), bytes->begin() + processedSize);
            processedSize += contiguousBytes.size();
        }

        bytes->shrink_from_back_to(processedSize);

        uint32_t messageSize = infra::MaxVarIntSize(this->forwardingServiceId) + infra::MaxVarIntSize((methodId << 3) | 2) + infra::MaxVarIntSize(bytes->size()) + bytes->size();

        RequestSend([this, methodId]()
            {
                infra::DataOutputStream::WithErrorPolicy stream(services::ServiceProxy::Rpc().SendStreamWriter());
                infra::ProtoFormatter formatter(stream);
                formatter.PutVarInt(this->forwardingServiceId);
                formatter.PutBytesField(*bytes, methodId);
                bytes = infra::none;

                services::ServiceProxy::Rpc().Send();
                MethodDone();
            },
            messageSize);
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
