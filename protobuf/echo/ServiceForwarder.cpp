#include "protobuf/echo/ServiceForwarder.hpp"

namespace services
{
    ServiceForwarderBase::ServiceForwarderBase(infra::ByteRange messageBuffer, Echo& echo, Echo& forwardTo)
        : Service(echo)
        , ServiceProxy(forwardTo, messageBuffer.size())
        , messageBuffer(messageBuffer)
    {}

    void ServiceForwarderBase::Handle(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy)
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

        RequestSend([this, methodId]()
            {
                infra::DataOutputStream::WithErrorPolicy stream(services::ServiceProxy::Rpc().SendStreamWriter());
                infra::ProtoFormatter formatter(stream);
                formatter.PutVarInt(this->forwardingServiceId);
                formatter.PutBytesField(*bytes, methodId);
                bytes = infra::none;

                services::ServiceProxy::Rpc().Send();
                MethodDone();
            });
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
