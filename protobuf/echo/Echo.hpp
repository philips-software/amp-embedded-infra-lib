#ifndef PROTOBUF_ECHO_HPP
#define PROTOBUF_ECHO_HPP

#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "services/network/Connection.hpp"
#include "services/util/MessageCommunication.hpp"

namespace services
{
    class Service;
    class ServiceProxy;

    struct ProtoBool {};
    struct ProtoUInt32 {};
    struct ProtoInt32 {};
    struct ProtoUInt64 {};
    struct ProtoInt64 {};
    struct ProtoFixed32 {};
    struct ProtoFixed64 {};
    struct ProtoSFixed32 {};
    struct ProtoSFixed64 {};
    struct ProtoStdString {};

    template<class T>
    struct ProtoMessage {};

    template<class T>
    struct ProtoEnum {};

    template<std::size_t Max>
    struct ProtoBytes {};

    template<std::size_t Max>
    struct ProtoString {};

    template<std::size_t Max, class T>
    struct ProtoRepeated {};

    template<class T>
    class ProtoValueType;

    template<>
    class ProtoValueType<ProtoUInt32>
    {
        using Type = uint32_t;
    };

    class EchoErrorPolicy
    {
    protected:
        ~EchoErrorPolicy() = default;

    public:
        virtual void MessageFormatError() = 0;
        virtual void ServiceNotFound(uint32_t serviceId) = 0;
        virtual void MethodNotFound(uint32_t serviceId, uint32_t methodId) = 0;
    };

    class EchoErrorPolicyAbort
        : public EchoErrorPolicy
    {
    public:
        virtual void MessageFormatError() override;
        virtual void ServiceNotFound(uint32_t serviceId) override;
        virtual void MethodNotFound(uint32_t serviceId, uint32_t methodId) override;
    };

    extern EchoErrorPolicyAbort echoErrorPolicyAbort;

    class Echo
    {
    public:
        virtual void RequestSend(ServiceProxy& serviceProxy) = 0;
        virtual infra::StreamWriter& SendStreamWriter() = 0;
        virtual void Send() = 0;
        virtual void ServiceDone(Service& service) = 0;
        virtual void AttachService(Service& service) = 0;
        virtual void DetachService(Service& service) = 0;
    };

    class Service
        : public infra::IntrusiveList<Service>::NodeType
    {
    public:
        Service(Echo& echo, uint32_t id);
        Service(const Service& other) = delete;
        Service& operator=(const Service& other) = delete;
        ~Service();

        void MethodDone();
        uint32_t ServiceId() const;
        bool InProgress() const;
        void HandleMethod(uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy);

    protected:
        Echo& Rpc();
        virtual void Handle(uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy) = 0;

    private:
        Echo& echo;
        uint32_t serviceId;
        bool inProgress = false;
    };

    class ServiceProxy
        : public infra::IntrusiveList<ServiceProxy>::NodeType
    {
    public:
        ServiceProxy(Echo& echo, uint32_t id, uint32_t maxMessageSize);

        Echo& Rpc();
        void RequestSend(infra::Function<void()> onGranted);
        void GrantSend();
        uint32_t MaxMessageSize() const;

    private:
        Echo& echo;
        uint32_t serviceId;
        uint32_t maxMessageSize;
        infra::Function<void()> onGranted;
    };

    class ServiceForwarder
        : public services::Service
        , private services::ServiceProxy
    {
    public:
        ServiceForwarder(infra::ByteRange messageBuffer, Echo& echo, uint32_t id, Echo& forwardTo);

        template<std::size_t MaxMessageSize>
            using WithMaxMessageSize = infra::WithStorage<ServiceForwarder, std::array<uint8_t, MaxMessageSize>>;

        virtual void Handle(uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy) override;

    private:
        const infra::ByteRange messageBuffer;
        infra::Optional<infra::ByteRange> bytes;
        uint32_t methodId;
    };

    class EchoOnStreams
        : public Echo
        , public infra::EnableSharedFromThis<EchoOnStreams>
    {
    public:
        EchoOnStreams(EchoErrorPolicy& errorPolicy = echoErrorPolicyAbort);

        // Implementation of Echo
        virtual void RequestSend(ServiceProxy& serviceProxy) override;
        virtual infra::StreamWriter& SendStreamWriter() override;
        virtual void Send() override;
        virtual void ServiceDone(Service& service) override;
        virtual void AttachService(Service& service) override;
        virtual void DetachService(Service& service) override;

    protected:
        virtual void RequestSendStream(std::size_t size) = 0;
        virtual void BusyServiceDone() = 0;

        void ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents);
        void SetStreamWriter(infra::SharedPtr<infra::StreamWriter>&& writer);
        bool ServiceBusy() const;
        bool ProcessMessage(infra::DataInputStream& stream);

    protected:
        EchoErrorPolicy& errorPolicy;

    private:
        infra::IntrusiveList<Service> services;
        infra::SharedPtr<infra::StreamWriter> streamWriter;
        infra::IntrusiveList<ServiceProxy> sendRequesters;
        infra::Optional<uint32_t> serviceBusy;
    };

    class EchoOnConnection
        : public EchoOnStreams
        , public ConnectionObserver
    {
    public:
        using EchoOnStreams::EchoOnStreams;

        // Implementation of ConnectionObserver
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

    protected:
        virtual void RequestSendStream(std::size_t size) override;
        virtual void BusyServiceDone() override;
    };

    class EchoOnMessageCommunication
        : public EchoOnStreams
        , public MessageCommunicationObserver
    {
    public:
        EchoOnMessageCommunication(MessageCommunication& subject, EchoErrorPolicy& errorPolicy = echoErrorPolicyAbort);

        // Implementation of MessageCommunicationObserver
        virtual void SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader) override;

        virtual void ServiceDone(Service& service) override;

    protected:
        virtual void RequestSendStream(std::size_t size) override;
        virtual void BusyServiceDone() override;

    private:
        void ProcessMessage();

    private:
        infra::SharedPtr<infra::StreamReaderWithRewinding> reader;
    };
}

#endif
