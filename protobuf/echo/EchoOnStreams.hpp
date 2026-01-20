#ifndef PROTOBUF_ECHO_ON_STREAMS_HPP
#define PROTOBUF_ECHO_ON_STREAMS_HPP

#include "infra/stream/BufferingStreamReader.hpp"
#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Function.hpp"
#include "protobuf/echo/Echo.hpp"
#include "protobuf/echo/Serialization.hpp"
#include <optional>

namespace services
{
    class EchoOnStreams
        : public EchoWithPolicy
    {
    public:
        explicit EchoOnStreams(services::MethodSerializerFactory& serializerFactory, const EchoErrorPolicy& errorPolicy = echoErrorPolicyAbortOnMessageFormatError);
        ~EchoOnStreams();

        // Implementation of EchoWithPolicy
        void SetPolicy(EchoPolicy& policy) override;
        void RequestSend(ServiceProxy& serviceProxy) override;
        void ServiceDone() override;
        void CancelRequestSend(ServiceProxy& serviceProxy) override;
        services::MethodSerializerFactory& SerializerFactory() override;

    protected:
        virtual infra::SharedPtr<MethodSerializer> GrantSend(ServiceProxy& proxy);
        virtual infra::SharedPtr<MethodDeserializer> StartingMethod(uint32_t serviceId, uint32_t methodId, infra::SharedPtr<MethodDeserializer>&& deserializer);
        virtual void RequestSendStream(std::size_t size) = 0;

        void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer);
        void DataReceived(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);
        void ReleaseReader();
        void Initialized();
        virtual void MethodContents(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader);
        virtual void ReleaseDeserializer();

    private:
        void TryGrantSend();

        void DataReceived();
        void StartReceiveMessage();
        void ContinueReceiveMessage();
        void StartMethod(uint32_t serviceId, uint32_t methodId, uint32_t size);
        void LimitedReaderDone();

    private:
        static EchoPolicy defaultPolicy;

        services::MethodSerializerFactory& serializerFactory;
        const EchoErrorPolicy& errorPolicy;
        EchoPolicy* policy = &defaultPolicy;

        infra::IntrusiveList<ServiceProxy> sendRequesters;
        ServiceProxy* sendingProxy = nullptr;
        infra::SharedPtr<MethodSerializer> methodSerializer;
        bool partlySent = false;
        bool skipNextStream = false;

        infra::SharedPtr<infra::StreamReaderWithRewinding> readerPtr;
        std::optional<infra::LimitedStreamReaderWithRewinding> limitedReader;
        infra::SharedPtr<MethodDeserializer> methodDeserializer;
        infra::BoundedDeque<uint8_t>::WithMaxSize<32> receiveBuffer;
        std::optional<infra::BufferingStreamReader> bufferedReader;
        infra::AccessedBySharedPtr limitedReaderAccess;

        infra::SharedOptional<MethodDeserializerDummy> deserializerDummy;

        bool delayDataReceived = false;
        bool delayedDataReceived = false;
    };
}

#endif
