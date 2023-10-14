#ifndef PROTOBUF_ECHO_HPP
#define PROTOBUF_ECHO_HPP

#include "infra/stream/BufferingStreamReader.hpp"
#include "infra/stream/CountingOutputStream.hpp"
#include "infra/stream/LimitedInputStream.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Compatibility.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
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

        virtual void MethodContents(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader) = 0;
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
        void StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size);
        void ReaderDone();

    private:
        EchoErrorPolicy& errorPolicy;

        infra::IntrusiveList<ServiceProxy> sendRequesters;
        ServiceProxy* sendingProxy = nullptr;
        infra::SharedPtr<MethodSerializer> methodSerializer;
        uint32_t sendingServiceId;
        uint32_t sendingMethodId;

        infra::SharedPtr<infra::StreamReaderWithRewinding> readerPtr;
        infra::Optional<infra::LimitedStreamReaderWithRewinding> limitedReader;
        infra::SharedPtr<MethodDeserializer> methodDeserializer;
        infra::BoundedDeque<uint8_t>::WithMaxSize<32> receiveBuffer;
        infra::Optional<infra::BufferingStreamReader> bufferedReader;
        infra::AccessedBySharedPtr readerAccess;
    };

    template<class Message, class Service, class... Args>
    class MethodDeserializerImpl
        : public MethodDeserializer
    {
    public:
        MethodDeserializerImpl(Service& service, void (Service::*method)(Args...));

        void MethodContents(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

    private:
        template<std::size_t... I>
        void Execute(std::index_sequence<I...>);

    private:
        ProtoMessageReceiver<Message> receiver;
        Service& service;
        void (Service::*method)(Args...);
    };

    class MethodDeserializerDummy
        : public MethodDeserializer
    {
    public:
        MethodDeserializerDummy(Echo& echo);

        void MethodContents(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader) override;
        void ExecuteMethod() override;
        bool Failed() const override;

    private:
        Echo& echo;
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

    template<class Message, class Service, class... Args>
    MethodDeserializerImpl<Message, Service, Args...>::MethodDeserializerImpl(Service& service, void (Service::*method)(Args...))
        : service(service)
        , method(method)
    {}

    template<class Message, class Service, class... Args>
    void MethodDeserializerImpl<Message, Service, Args...>::MethodContents(const infra::SharedPtr<infra::StreamReaderWithRewinding>& reader)
    {
        receiver.Feed(*reader);
    }

    template<class Message, class Service, class... Args>
    void MethodDeserializerImpl<Message, Service, Args...>::ExecuteMethod()
    {
        Execute(std::make_index_sequence<sizeof...(Args)>{});
    }

    template<class Message, class Service, class... Args>
    bool MethodDeserializerImpl<Message, Service, Args...>::Failed() const
    {
        return receiver.Failed();
    }

    template<class Message, class Service, class... Args>
    template<std::size_t... I>
    void MethodDeserializerImpl<Message, Service, Args...>::Execute(std::index_sequence<I...>)
    {
        (service.*method)(receiver.message.Get(std::integral_constant<uint32_t, I>{})...);
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
        return stream.Failed();
    }
}

#endif
