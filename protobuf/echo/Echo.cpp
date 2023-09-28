#include "protobuf/echo/Echo.hpp"
#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/stream/BufferingStreamReader.hpp"

namespace services
{
    EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    EchoErrorPolicyAbort echoErrorPolicyAbort;

    void Service::MethodDone()
    {
        Rpc().ServiceDone(*this);
    }

    Echo& Service::Rpc()
    {
        return Subject();
    }

    ServiceProxy::ServiceProxy(Echo& echo, uint32_t maxMessageSize)
        : echo(echo)
        , maxMessageSize(maxMessageSize)
    {}

    Echo& ServiceProxy::Rpc()
    {
        return echo;
    }

    void ServiceProxy::RequestSend(infra::Function<void()> onGranted)
    {
        RequestSend(onGranted, MaxMessageSize());
    }

    void ServiceProxy::RequestSend(infra::Function<void()> onGranted, uint32_t requestedSize)
    {
        this->onGranted = onGranted;
        currentRequestedSize = requestedSize;
        echo.RequestSend(*this);
    }

    infra::SharedPtr<MethodSerializer> ServiceProxy::GrantSend()
    {
        onGranted();
        return methodSerializer;
    }

    uint32_t ServiceProxy::MaxMessageSize() const
    {
        return maxMessageSize;
    }

    uint32_t ServiceProxy::CurrentRequestedSize() const
    {
        return currentRequestedSize;
    }

    void ServiceProxy::SetSerializer(const infra::SharedPtr<MethodSerializer>& serializer)
    {
        methodSerializer = serializer;
    }

    void EchoErrorPolicyAbortOnMessageFormatError::MessageFormatError()
    {
        std::abort();
    }

    void EchoErrorPolicyAbortOnMessageFormatError::ServiceNotFound(uint32_t serviceId)
    {}

    void EchoErrorPolicyAbortOnMessageFormatError::MethodNotFound(uint32_t serviceId, uint32_t methodId)
    {}

    void EchoErrorPolicyAbort::ServiceNotFound(uint32_t serviceId)
    {
        std::abort();
    }

    void EchoErrorPolicyAbort::MethodNotFound(uint32_t serviceId, uint32_t methodId)
    {
        std::abort();
    }

    EchoOnStreams::EchoOnStreams(EchoErrorPolicy& errorPolicy)
        : errorPolicy(errorPolicy)
    {}

    void EchoOnStreams::RequestSend(ServiceProxy& serviceProxy)
    {
        sendRequesters.push_back(serviceProxy);

        TryGrantSend();
    }

    void EchoOnStreams::ServiceDone(Service& service)
    {
        methodDeserializer = nullptr;

        if (readerPtr != nullptr)
            DataReceived();
    }

    void EchoOnStreams::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        readerPtr = std::move(reader);
        DataReceived();
    }

    void EchoOnStreams::TryGrantSend()
    {
        if (sendingProxy == nullptr && !sendRequesters.empty())
        {
            sendingProxy = &sendRequesters.front();
            sendRequesters.pop_front();
            RequestSendStream(sendingProxy->CurrentRequestedSize());
        }
    }

    void EchoOnStreams::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        if (methodSerializer == nullptr)
            methodSerializer = sendingProxy->GrantSend();

        auto more = methodSerializer->SendStreamAvailable(std::move(writer));

        if (more)
            RequestSendStream(sendingProxy->CurrentRequestedSize());
        else
        {
            methodSerializer = nullptr;
            TryGrantSend();
        }
    }

    void EchoOnStreams::DataReceived()
    {
        infra::BufferingStreamReader bufferedReader{ receiveBuffer, *readerPtr };

        while (methodDeserializer == nullptr || limitedReader != infra::none)
        {
            if (limitedReader == infra::none)
            {
                infra::DataInputStream::WithErrorPolicy stream(bufferedReader, infra::softFail);
                infra::StreamErrorPolicy formatErrorPolicy(infra::softFail);
                infra::ProtoParser parser(stream, formatErrorPolicy);
                uint32_t serviceId = static_cast<uint32_t>(parser.GetVarInt());
                auto [contents, methodId] = parser.GetPartialField();
                if (stream.Failed())
                {
                    bufferedReader.Rewind(0);
                    return;
                }

                if (formatErrorPolicy.Failed() || !contents.Is<infra::PartialProtoLengthDelimited>())
                    errorPolicy.MessageFormatError();
                else
                {
                    limitedReader.Emplace(bufferedReader, contents.Get<infra::PartialProtoLengthDelimited>().length);
                    StartMethod(serviceId, methodId);

                    if (formatErrorPolicy.Failed())
                        errorPolicy.MessageFormatError();
                }
            }

            if (limitedReader != infra::none)
            {
                limitedReader->SwitchInput(bufferedReader);
                methodDeserializer->MethodContents(*limitedReader);
            }

            if (limitedReader->LimitReached())
            {
                limitedReader = infra::none;
                methodDeserializer->ExecuteMethod();
            }
        }

        if (methodDeserializer == nullptr)
            readerPtr = nullptr;
    }

    void EchoOnStreams::StartMethod(uint32_t serviceId, uint32_t methodId)
    {
        if (!NotifyObservers([this, serviceId, methodId](auto& service)
            {
                if (service.AcceptsService(serviceId))
                {
                    methodDeserializer = service.StartMethod(serviceId, methodId, errorPolicy);
                    return true;
                }

                return false;
            }))
        {
            errorPolicy.ServiceNotFound(serviceId);
            methodDeserializer = infra::MakeSharedOnHeap<MethodDeserializerDummy>();
        }
    }

    void MethodDeserializerDummy::MethodContents(infra::StreamReaderWithRewinding& reader)
    {
        while (!reader.Empty())
            reader.ExtractContiguousRange(std::numeric_limits<uint32_t>::max());
    }

    void MethodDeserializerDummy::ExecuteMethod()
    {}
}
