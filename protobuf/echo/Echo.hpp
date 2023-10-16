#ifndef PROTOBUF_ECHO_HPP
#define PROTOBUF_ECHO_HPP

#include "infra/stream/BufferingStreamReader.hpp"
#include "infra/stream/CountingOutputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Compatibility.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include "infra/util/ReallyAssert.hpp"
#include "infra/util/SharedOptional.hpp"
#include "protobuf/echo/Proto.hpp"
#include "protobuf/echo/ProtoMessageReceiver.hpp"
#include "protobuf/echo/ProtoMessageSender.hpp"
#include "services/util/MessageCommunication.hpp"

namespace services
{
    class Echo;
    class Service;
    class ServiceProxy;

    class EchoErrorPolicy
    {
    protected:
        ~EchoErrorPolicy() = default;

    public:
        virtual void MessageFormatError() = 0;
        virtual void ServiceNotFound(uint32_t serviceId) = 0;
        virtual void MethodNotFound(uint32_t serviceId, uint32_t methodId) = 0;
    };

    class EchoErrorPolicyAbortOnMessageFormatError
        : public EchoErrorPolicy
    {
    public:
        void MessageFormatError() override;
        void ServiceNotFound(uint32_t serviceId) override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) override;
    };

    class EchoErrorPolicyAbort
        : public EchoErrorPolicyAbortOnMessageFormatError
    {
    public:
        void ServiceNotFound(uint32_t serviceId) override;
        void MethodNotFound(uint32_t serviceId, uint32_t methodId) override;
    };

    extern EchoErrorPolicyAbortOnMessageFormatError echoErrorPolicyAbortOnMessageFormatError;
    extern EchoErrorPolicyAbort echoErrorPolicyAbort;

    class MethodDeserializer
    {
    public:
        MethodDeserializer() = default;
        MethodDeserializer(const MethodDeserializer& other) = delete;
        MethodDeserializer& operator=(const MethodDeserializer& other) = delete;
        virtual ~MethodDeserializer() = default;

        virtual void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) = 0;
        virtual void ExecuteMethod() = 0;
        virtual bool Failed() const = 0;
    };

    class MethodSerializer
    {
    public:
        MethodSerializer() = default;
        MethodSerializer(const MethodSerializer& other) = delete;
        MethodSerializer& operator=(const MethodSerializer& other) = delete;
        virtual ~MethodSerializer() = default;

        virtual bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) = 0;

        virtual void SerializationDone()
        {}
    };

    class Service
        : public infra::Observer<Service, Echo>
    {
    public:
        using infra::Observer<Service, Echo>::Observer;

        virtual bool AcceptsService(uint32_t id) const = 0;

        void MethodDone();
        virtual infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, EchoErrorPolicy& errorPolicy) = 0;

    protected:
        Echo& Rpc();
    };

    class ServiceProxy
        : public infra::IntrusiveList<ServiceProxy>::NodeType
    {
    public:
        ServiceProxy(Echo& echo, uint32_t maxMessageSize);

        Echo& Rpc();
        virtual void RequestSend(infra::Function<void()> onGranted);
        virtual void RequestSend(infra::Function<void()> onGranted, uint32_t requestedSize);
        virtual infra::SharedPtr<MethodSerializer> GrantSend();
        uint32_t MaxMessageSize() const;
        uint32_t CurrentRequestedSize() const;
        void SetSerializer(const infra::SharedPtr<MethodSerializer>& serializer);

    private:
        Echo& echo;
        uint32_t maxMessageSize;
        infra::Function<void()> onGranted;
        uint32_t currentRequestedSize = 0;
        infra::SharedPtr<MethodSerializer> methodSerializer;
    };

    class Echo
        : public infra::Subject<Service>
    {
    public:
        virtual void RequestSend(ServiceProxy& serviceProxy) = 0;
        virtual void ServiceDone() = 0;
    };

    template<class ServiceProxyType>
    class ServiceProxyResponseQueue
        : public ServiceProxyType
    {
    public:
        struct Request
        {
            infra::Function<void()> onRequestGranted;
            uint32_t requestedSize;
        };

        using Container = infra::BoundedDeque<Request>;

        template<std::size_t Max>
        using WithStorage = infra::WithStorage<ServiceProxyResponseQueue, typename Container::template WithMaxSize<Max>>;

        template<class... Args>
        explicit ServiceProxyResponseQueue(Container& container, Args&&... args);

        void RequestSend(infra::Function<void()> onRequestGranted) override;
        void RequestSend(infra::Function<void()> onRequestGranted, uint32_t requestedSize) override;

    private:
        void ProcessSendQueue();

    private:
        Container& container;

        bool responseInProgress{ false };
    };

    class MethodDeserializerDummy
        : public MethodDeserializer
    {
    public:
        explicit MethodDeserializerDummy(Echo& echo);

        void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

    private:
        Echo& echo;
    };

    class EchoOnStreams
        : public Echo
        , public infra::EnableSharedFromThis<EchoOnStreams>
    {
    public:
        explicit EchoOnStreams(EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);
        ~EchoOnStreams() override;

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;
        void ServiceDone() override;

    protected:
        virtual infra::SharedPtr<MethodSerializer> GrantSend(ServiceProxy& proxy);
        virtual infra::SharedPtr<MethodDeserializer> StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const infra::SharedPtr<MethodDeserializer>& deserializer);
        virtual void RequestSendStream(std::size_t size) = 0;
        virtual void AckReceived() = 0;

        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer);
        void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);

    private:
        void TryGrantSend();

        void DataReceived();
        void StartReceiveMessage();
        void ContinueReceiveMessage();
        void StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size);
        void ReaderDone();

    private:
        EchoErrorPolicy& errorPolicy;

        infra::IntrusiveList<ServiceProxy> sendRequesters;
        ServiceProxy* sendingProxy = nullptr;
        infra::SharedPtr<MethodSerializer> methodSerializer;

        infra::SharedPtr<infra::StreamReaderWithRewinding> readerPtr;
        infra::Optional<infra::LimitedStreamReaderWithRewinding> limitedReader;
        infra::SharedPtr<MethodDeserializer> methodDeserializer;
        infra::BoundedDeque<uint8_t>::WithMaxSize<32> receiveBuffer;
        infra::Optional<infra::BufferingStreamReader> bufferedReader;
        infra::AccessedBySharedPtr readerAccess;

        infra::SharedOptional<MethodDeserializerDummy> deserializerDummy;
    };

    template<class Message, class... Args>
    class MethodDeserializerImpl
        : public MethodDeserializer
    {
    public:
        MethodDeserializerImpl(const infra::Function<void(Args...)>& method);

        void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

    private:
        template<std::size_t... I>
        void Execute(std::index_sequence<I...>);

    private:
        ProtoMessageReceiver<Message> receiver;
        infra::Function<void(Args...)> method;
    };

    class MethodDeserializerFactory
    {
    public:
        template<class Message>
        class ForMessage;

        MethodDeserializerFactory() = default;
        MethodDeserializerFactory(const MethodDeserializerFactory& other) = delete;
        MethodDeserializerFactory& operator=(const MethodDeserializerFactory& other) = delete;
        virtual ~MethodDeserializerFactory() = default;

        virtual infra::SharedPtr<infra::ByteRange> DeserializerMemory(uint32_t size) = 0;

        template<class Message, class... Args>
        infra::SharedPtr<MethodDeserializer> MakeDeserializer(const infra::Function<void(Args...)>& method)
        {
            using Deserializer = MethodDeserializerImpl<Message, Args...>;

            auto memory = DeserializerMemory(sizeof(Deserializer));
            auto deserializer = new (memory->begin()) Deserializer(method);
            return infra::MakeContainedSharedObject(*deserializer, memory);
        }
    };

    template<class Message>
    class MethodDeserializerFactory::ForMessage
        : public MethodDeserializerFactory
    {
    public:
        infra::SharedPtr<infra::ByteRange> DeserializerMemory(uint32_t size) override
        {
            really_assert(size <= sizeof(MethodDeserializerImpl<Message>));
            return access.MakeShared(infra::Head(infra::MakeRange(storage), size));
        }

    private:
        alignas(MethodDeserializerImpl<Message>) std::array<uint8_t, sizeof(MethodDeserializerImpl<Message>)> storage;
        infra::AccessedBySharedPtr access{ infra::emptyFunction };
    };

    template<class Message, class... Args>
    class MethodSerializerImpl
        : public MethodSerializer
    {
    public:
        MethodSerializerImpl(uint32_t serviceId, uint32_t methodId, Args... args);

        bool Serialize(infra::SharedPtr<infra::StreamWriter>&& writer) override;

    private:
        uint32_t serviceId;
        uint32_t methodId;
        bool headerSent = false;
        Message message;
        ProtoMessageSender<Message> sender{ message };
    };

    ////    Implementation    ////

    template<class ServiceProxyType>
    template<class... Args>
    ServiceProxyResponseQueue<ServiceProxyType>::ServiceProxyResponseQueue(Container& container, Args&&... args)
        : ServiceProxyType{ std::forward<Args>(args)... }
        , container{ container }
    {}

    template<class ServiceProxyType>
    void ServiceProxyResponseQueue<ServiceProxyType>::RequestSend(infra::Function<void()> onRequestGranted)
    {
        RequestSend(onRequestGranted, ServiceProxyType::MaxMessageSize());
    }

    template<class ServiceProxyType>
    void ServiceProxyResponseQueue<ServiceProxyType>::RequestSend(infra::Function<void()> onRequestGranted, uint32_t requestedSize)
    {
        if (container.full())
            return;

        container.push_back({ onRequestGranted, requestedSize });
        ProcessSendQueue();
    }

    template<class ServiceProxyType>
    void ServiceProxyResponseQueue<ServiceProxyType>::ProcessSendQueue()
    {
        if (!responseInProgress && !container.empty())
        {
            responseInProgress = true;
            ServiceProxyType::RequestSend([this]
                {
                    container.front().onRequestGranted();
                    container.pop_front();

                    responseInProgress = false;
                    ProcessSendQueue();
                },
                container.front().requestedSize);
        }
    }

    template<class Message, class... Args>
    MethodDeserializerImpl<Message, Args...>::MethodDeserializerImpl(const infra::Function<void(Args...)>& method)
        : method(method)
    {}

    template<class Message, class... Args>
    void MethodDeserializerImpl<Message, Args...>::MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        receiver.Feed(*reader);
    }

    template<class Message, class... Args>
    void MethodDeserializerImpl<Message, Args...>::ExecuteMethod()
    {
        Execute(std::make_index_sequence<sizeof...(Args)>{});
    }

    template<class Message, class... Args>
    bool MethodDeserializerImpl<Message, Args...>::Failed() const
    {
        return receiver.Failed();
    }

    template<class Message, class... Args>
    template<std::size_t... I>
    void MethodDeserializerImpl<Message, Args...>::Execute(std::index_sequence<I...>)
    {
        method(receiver.message.Get(std::integral_constant<uint32_t, I>{})...);
    }

    template<class Message, class... Args>
    MethodSerializerImpl<Message, Args...>::MethodSerializerImpl(uint32_t serviceId, uint32_t methodId, Args... args)
        : serviceId(serviceId)
        , methodId(methodId)
        , message(args...)
    {}

    template<class Message, class... Args>
    bool MethodSerializerImpl<Message, Args...>::Serialize(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        infra::DataOutputStream::WithErrorPolicy stream(*writer, infra::softFail);

        if (!headerSent)
        {
            infra::DataOutputStream::WithWriter<infra::CountingStreamWriter> countingStream;
            infra::ProtoFormatter countingFormatter{ countingStream };
            message.Serialize(countingFormatter);

            infra::ProtoFormatter formatter(stream);
            formatter.PutVarInt(serviceId);
            formatter.PutLengthDelimitedSize(countingStream.Writer().Processed(), methodId);

            headerSent = true;
        }

        sender.Fill(stream);
        writer = nullptr;
        return stream.Failed();
    }
}

#endif
