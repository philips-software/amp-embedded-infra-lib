#include "protobuf/echo/Echo.hpp"
#include "infra/event/EventDispatcherWithWeakPtr.hpp"

namespace services
{
    EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    EchoErrorPolicyAbort echoErrorPolicyAbort;

    void Service::MethodDone()
    {
        Rpc().ServiceDone();
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

    EchoOnStreams::~EchoOnStreams()
    {
        readerAccess.SetAction(nullptr);
    }

    void EchoOnStreams::RequestSend(ServiceProxy& serviceProxy)
    {
        sendRequesters.push_back(serviceProxy);

        TryGrantSend();
    }

    void EchoOnStreams::ServiceDone()
    {
        methodDeserializer = nullptr;

        if (readerPtr != nullptr)
            DataReceived();
    }

    void EchoOnStreams::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        assert(readerPtr == nullptr);
        readerPtr = std::move(reader);
        bufferedReader.Emplace(receiveBuffer, *readerPtr);
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

    infra::SharedPtr<MethodSerializer> EchoOnStreams::GrantSend(ServiceProxy& proxy)
    {
        return proxy.GrantSend();
    }

    infra::SharedPtr<MethodDeserializer> EchoOnStreams::StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const infra::SharedPtr<MethodDeserializer>& deserializer)
    {
        return deserializer;
    }

    void EchoOnStreams::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        if (methodSerializer == nullptr)
            methodSerializer = GrantSend(*sendingProxy);

        auto more = methodSerializer->Serialize(std::move(writer));

        if (more)
            RequestSendStream(sendingProxy->CurrentRequestedSize());
        else
        {
            sendingProxy = nullptr;
            methodSerializer->SerializationDone();
            methodSerializer = nullptr;
            TryGrantSend();
        }
    }

    void EchoOnStreams::DataReceived()
    {
        while (readerPtr != nullptr && methodDeserializer == nullptr && !readerAccess.Referenced())
        {
            if (limitedReader == infra::none)
            {
                auto start = bufferedReader->ConstructSaveMarker();
                infra::DataInputStream::WithErrorPolicy stream(*bufferedReader, infra::softFail);
                infra::StreamErrorPolicy formatErrorPolicy(infra::softFail);
                infra::ProtoParser parser(stream, formatErrorPolicy);
                uint32_t serviceId = static_cast<uint32_t>(parser.GetVarInt());
                auto [contents, methodId] = parser.GetPartialField();
                if (stream.Failed())
                {
                    bufferedReader->Rewind(start);
                    bufferedReader = infra::none;
                    readerPtr = nullptr;
                    break;
                }

                if (formatErrorPolicy.Failed() || !contents.Is<infra::PartialProtoLengthDelimited>())
                    errorPolicy.MessageFormatError();
                else
                {
                    limitedReader.Emplace(*bufferedReader, contents.Get<infra::PartialProtoLengthDelimited>().length);
                    StartMethod(serviceId, methodId, contents.Get<infra::PartialProtoLengthDelimited>().length);

                    if (formatErrorPolicy.Failed())
                        errorPolicy.MessageFormatError();
                }
            }

            if (limitedReader != infra::none)
            {
                limitedReader->SwitchInput(*bufferedReader);
                readerAccess.SetAction(infra::emptyFunction);
                methodDeserializer->MethodContents(readerAccess.MakeShared(*limitedReader));

                if (readerAccess.Referenced())
                    readerAccess.SetAction([this]()
                        {
                            ReaderDone();
                            DataReceived();
                        });
                else
                    ReaderDone();
            }
            else
                AckReceived();
        }
    }

    void EchoOnStreams::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size)
    {
        if (!NotifyObservers([this, serviceId, methodId, size](auto& service)
                {
                    if (service.AcceptsService(serviceId))
                    {
                        methodDeserializer = StartingMethod(serviceId, methodId, size, service.StartMethod(serviceId, methodId, size, errorPolicy));
                        return true;
                    }

                    return false;
                }))
        {
            errorPolicy.ServiceNotFound(serviceId);
            methodDeserializer = infra::MakeSharedOnHeap<MethodDeserializerDummy>(*this);
        }
    }

    void EchoOnStreams::ReaderDone()
    {
        AckReceived();

        if (limitedReader->LimitReached())
        {
            limitedReader = infra::none;
            if (methodDeserializer->Failed())
            {
                errorPolicy.MessageFormatError();
                methodDeserializer = nullptr;
            }
            else
                methodDeserializer->ExecuteMethod();
        }
    }

    MethodDeserializerDummy::MethodDeserializerDummy(Echo& echo)
        : echo(echo)
    {}

    void MethodDeserializerDummy::MethodContents(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
    {
        while (!reader->Empty())
            reader->ExtractContiguousRange(std::numeric_limits<uint32_t>::max());
    }

    void MethodDeserializerDummy::ExecuteMethod()
    {
        echo.ServiceDone();
    }

    bool MethodDeserializerDummy::Failed() const
    {
        return false;
    }
}
