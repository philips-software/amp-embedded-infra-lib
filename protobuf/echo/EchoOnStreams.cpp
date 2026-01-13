#include "protobuf/echo/EchoOnStreams.hpp"

namespace services
{
    EchoPolicy EchoOnStreams::defaultPolicy;

    EchoOnStreams::EchoOnStreams(services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy)
        : serializerFactory(serializerFactory)
        , errorPolicy(errorPolicy)
    {}

    EchoOnStreams::~EchoOnStreams()
    {
        limitedReaderAccess.SetAction(infra::emptyFunction);
    }

    void EchoOnStreams::SetPolicy(EchoPolicy& policy)
    {
        this->policy = &policy;
    }

    void EchoOnStreams::RequestSend(ServiceProxy& serviceProxy)
    {
        policy->RequestSend(serviceProxy, [this](services::ServiceProxy& proxy)
            {
                assert(!sendRequesters.has_element(proxy));
                sendRequesters.push_back(proxy);

                TryGrantSend();
            });
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

        if (!delayDataReceived)
            DataReceived();
        else
            delayedDataReceived = true;
    }

    void EchoOnStreams::ReleaseReader()
    {
        limitedReaderAccess.SetAction(infra::emptyFunction);
        bufferedReader.reset();
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

        ReleaseReader();
        limitedReader.reset();
        ReleaseDeserializer();
    }

    void EchoOnStreams::ReleaseDeserializer()
    {
        methodDeserializer = nullptr;
    }

    void EchoOnStreams::TryGrantSend()
    {
        if (!skipNextStream && sendingProxy == nullptr && !sendRequesters.empty())
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
            {
                methodSerializer = GrantSend(*sendingProxy);
                policy->GrantingSend(*sendingProxy);
            }

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
        if (limitedReader != std::nullopt && readerPtr != nullptr)
            ContinueReceiveMessage();

        while (readerPtr != nullptr && methodDeserializer == nullptr && !limitedReaderAccess.Referenced())
        {
            if (limitedReader == std::nullopt)
                StartReceiveMessage();

            if (limitedReader != std::nullopt && readerPtr != nullptr)
                ContinueReceiveMessage();
        }

        if (readerPtr != nullptr && readerPtr->Empty() && !limitedReaderAccess.Referenced())
        {
            bufferedReader.reset();
            readerPtr = nullptr;
        }
    }

    void EchoOnStreams::StartReceiveMessage()
    {
        auto start = bufferedReader->ConstructSaveMarker();
        infra::DataInputStream::WithErrorPolicy stream(*bufferedReader, infra::softFail);
        infra::StreamErrorPolicy formatErrorPolicy(infra::softFail);
        infra::ProtoParser parser(stream, formatErrorPolicy);
        auto serviceId = static_cast<uint32_t>(parser.GetVarInt());
        auto [contents, methodId] = parser.GetPartialField();

        if (stream.Failed())
        {
            // bufferedReader is rewound to the start, so that the remainder of the reader is buffered. When readerPtr is set to nullptr,
            // any data present in reader is not repeated the next time
            bufferedReader->Rewind(start);
            bufferedReader.reset();
            readerPtr = nullptr;
        }
        else if (formatErrorPolicy.Failed() || !std::holds_alternative<infra::PartialProtoLengthDelimited>(contents))
            errorPolicy.MessageFormatError();
        else
        {
            limitedReader.emplace(*bufferedReader, std::get<infra::PartialProtoLengthDelimited>(contents).length);
            StartMethod(serviceId, methodId, std::get<infra::PartialProtoLengthDelimited>(contents).length);

            if (formatErrorPolicy.Failed())
                errorPolicy.MessageFormatError();
        }
    }

    void EchoOnStreams::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        methodDeserializer->MethodContents(std::move(reader));
    }

    void EchoOnStreams::ContinueReceiveMessage()
    {
        limitedReader->SwitchInput(*bufferedReader);
        limitedReaderAccess.SetAction(infra::emptyFunction);
        MethodContents(limitedReaderAccess.MakeShared(*limitedReader));

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
        delayDataReceived = true;
        if (bufferedReader->Empty())
            ReleaseReader();

        if (limitedReader->LimitReached())
        {
            limitedReader.reset();
            if (methodDeserializer->Failed())
            {
                errorPolicy.MessageFormatError();
                ReleaseDeserializer();
            }
            else
                methodDeserializer->ExecuteMethod();
        }

        delayDataReceived = false;
        if (delayedDataReceived)
        {
            delayedDataReceived = false;
            DataReceived();
        }
    }
}
