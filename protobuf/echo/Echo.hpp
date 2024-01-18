#ifndef PROTOBUF_ECHO_HPP
#define PROTOBUF_ECHO_HPP

#include "infra/stream/BufferingStreamReader.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Compatibility.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Observer.hpp"
#include "infra/util/Optional.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"
#include "protobuf/echo/Proto.hpp"
#include "protobuf/echo/Serialization.hpp"

namespace services
{
    class Echo;
    class Service;
    class ServiceProxy;

    class Service
        : public infra::Observer<Service, Echo>
    {
    public:
        using infra::Observer<Service, Echo>::Observer;

        virtual bool AcceptsService(uint32_t id) const = 0;

        void MethodDone();
        virtual infra::SharedPtr<MethodDeserializer> StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const EchoErrorPolicy& errorPolicy) = 0;

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
        virtual services::MethodSerializerFactory& SerializerFactory() = 0;
    };

    class EchoOnStreams
        : public Echo
        , public infra::EnableSharedFromThis<EchoOnStreams>
    {
    public:
        explicit EchoOnStreams(services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);
        ~EchoOnStreams();

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;
        void ServiceDone() override;
        services::MethodSerializerFactory& SerializerFactory() override;

    protected:
        virtual infra::SharedPtr<MethodSerializer> GrantSend(ServiceProxy& proxy);
        virtual infra::SharedPtr<MethodDeserializer> StartingMethod(uint32_t serviceId, uint32_t methodId, uint32_t size, const infra::SharedPtr<MethodDeserializer>& deserializer);
        virtual void RequestSendStream(std::size_t size) = 0;
        virtual void AckReceived() = 0;

        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer);
        void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);
        void ReleaseReader();

    private:
        void TryGrantSend();

        void DataReceived();
        void StartReceiveMessage();
        void ContinueReceiveMessage();
        void StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size);
        void ReaderDone();

    private:
        services::MethodSerializerFactory& serializerFactory;
        const EchoErrorPolicy& errorPolicy;

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
}

#endif
