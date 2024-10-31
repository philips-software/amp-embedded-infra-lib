#include "protobuf/echo/Echo.hpp"

namespace services
{
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

    ServiceProxy::~ServiceProxy()
    {
        if (onGranted != nullptr)
            echo.CancelRequestSend(*this);
    }

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
        return std::move(methodSerializer);
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

    EchoOnStreams::EchoOnStreams(services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : serializerFactory(serializerFactory)
        , errorPolicy(errorPolicy)
    {}

    EchoOnStreams::~EchoOnStreams()
    {
        limitedReaderAccess.SetAction(nullptr);
    }

    void EchoOnStreams::RequestSend(ServiceProxy& serviceProxy)
    {
        assert(!sendRequesters.has_element(serviceProxy));
        sendRequesters.push_back(serviceProxy);

        TryGrantSend();
    }

    void EchoOnStreams::ServiceDone()
    {
        ReleaseDeserializer();

        if (readerPtr != nullptr)
            DataReceived();
    }

    void EchoOnStreams::CancelRequestSend(ServiceProxy& serviceProxy)
    {
        if (sendRequesters.has_element(serviceProxy))
            sendRequesters.erase(serviceProxy);
        else
        {
            assert(&serviceProxy == sendingProxy);
            sendingProxy = nullptr;
            skipNextStream = true;
        }
    }

    services::MethodSerializerFactory& EchoOnStreams::SerializerFactory()
    {
        return serializerFactory;
    }

    void EchoOnStreams::DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        really_assert(readerPtr == nullptr);

        readerPtr = std::move(reader);
        bufferedReader.emplace(receiveBuffer, *readerPtr);
        DataReceived();
    }

    void EchoOnStreams::ReleaseReader()
    {
        limitedReaderAccess.SetAction(nullptr);
        bufferedReader = infra::none;
        readerPtr = nullptr;
    }

    void EchoOnStreams::Initialized()
    {
        if (partlySent)
        {
            sendingProxy = nullptr;
            methodSerializer = nullptr;
            skipNextStream = true;
        }

        limitedReaderAccess.SetAction(infra::emptyFunction);
        bufferedReader = infra::none;
        readerPtr = nullptr;
        limitedReader = infra::none;
        ReleaseDeserializer();
    }

    void EchoOnStreams::ReleaseDeserializer()
    {
        methodDeserializer = nullptr;
    }

    void EchoOnStreams::TryGrantSend()
    {
        if (sendingProxy == nullptr && !sendRequesters.empty())
        {
            sendingProxy = &sendRequesters.front();
            sendRequesters.pop_front();
            RequestSendStream(sendingProxy->CurrentRequestedSize() + 2 * infra::MaxVarIntSize(std::numeric_limits<uint64_t>::max()));
        }
    }

    infra::SharedPtr<MethodSerializer> EchoOnStreams::GrantSend(ServiceProxy& proxy)
    {
        return proxy.GrantSend();
    }

    infra::SharedPtr<MethodDeserializer> EchoOnStreams::StartingMethod(uint32_t serviceId, uint32_t methodId, infra::SharedPtr<MethodDeserializer>&& deserializer)
    {
        return std::move(deserializer);
    }

    void EchoOnStreams::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        if (skipNextStream)
        {
            skipNextStream = false;
            TryGrantSend();
        }
        else
        {
            if (methodSerializer == nullptr)
                methodSerializer = GrantSend(*sendingProxy);

            partlySent = methodSerializer->Serialize(std::move(writer));

            if (partlySent)
                RequestSendStream(sendingProxy->CurrentRequestedSize());
            else
            {
                sendingProxy = nullptr;
                methodSerializer = nullptr;
                TryGrantSend();
            }
        }
    }

    void EchoOnStreams::DataReceived()
    {
        if (limitedReader != infra::none && readerPtr != nullptr)
            ContinueReceiveMessage();

        while (readerPtr != nullptr && methodDeserializer == nullptr && !limitedReaderAccess.Referenced())
        {
            if (limitedReader == infra::none)
                StartReceiveMessage();

            if (limitedReader != infra::none && readerPtr != nullptr)
                ContinueReceiveMessage();
        }

        if (readerPtr != nullptr && readerPtr->Empty() && !limitedReaderAccess.Referenced())
        {
            bufferedReader = infra::none;
            readerPtr = nullptr;
        }
    }

    void EchoOnStreams::StartReceiveMessage()
    {
        auto start = bufferedReader->ConstructSaveMarker();
        infra::DataInputStream::WithErrorPolicy stream(*bufferedReader, infra::softFail);
        infra::StreamErrorPolicy formatErrorPolicy(infra::softFail);
        infra::ProtoParser parser(stream, formatErrorPolicy);
        uint32_t serviceId = static_cast<uint32_t>(parser.GetVarInt());
        auto [contents, methodId] = parser.GetPartialField();

        if (stream.Failed())
        {
            // bufferedReader is rewound to the start, so that the remainder of the reader is buffered. When readerPtr is set to nullptr,
            // any data present in reader is not repeated the next time
            bufferedReader->Rewind(start);
            bufferedReader = infra::none;
            readerPtr = nullptr;
        }
        else if (formatErrorPolicy.Failed() || !contents.Is<infra::PartialProtoLengthDelimited>())
            errorPolicy.MessageFormatError();
        else
        {
            limitedReader.emplace(*bufferedReader, contents.Get<infra::PartialProtoLengthDelimited>().length);
            StartMethod(serviceId, methodId, contents.Get<infra::PartialProtoLengthDelimited>().length);

            if (formatErrorPolicy.Failed())
                errorPolicy.MessageFormatError();
        }
    }

    void EchoOnStreams::ContinueReceiveMessage()
    {
        limitedReader->SwitchInput(*bufferedReader);
        limitedReaderAccess.SetAction(infra::emptyFunction);
        methodDeserializer->MethodContents(limitedReaderAccess.MakeShared(*limitedReader));

        if (limitedReaderAccess.Referenced())
            limitedReaderAccess.SetAction([this]()
                {
                    auto& self = *this;
                    LimitedReaderDone();
                    // LimitedReaderDone() may result in limitedReaderAccess' completion callback being reset, which invalidates the saved this pointer
                    self.DataReceived();
                });
        else
            LimitedReaderDone();
    }

    void EchoOnStreams::StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size)
    {
        if (!NotifyObservers([this, serviceId, methodId, size](auto& service)
                {
                    if (service.AcceptsService(serviceId))
                    {
                        methodDeserializer = StartingMethod(serviceId, methodId, service.StartMethod(serviceId, methodId, size, errorPolicy));
                        return true;
                    }

                    return false;
                }))
        {
            errorPolicy.ServiceNotFound(serviceId);
            methodDeserializer = StartingMethod(serviceId, methodId, deserializerDummy.Emplace(*this));
        }
    }

    void EchoOnStreams::LimitedReaderDone()
    {
        if (limitedReader->LimitReached())
        {
            limitedReader = infra::none;
            if (methodDeserializer->Failed())
            {
                errorPolicy.MessageFormatError();
                ReleaseDeserializer();
            }
            else
                methodDeserializer->ExecuteMethod();
        }
    }
}
