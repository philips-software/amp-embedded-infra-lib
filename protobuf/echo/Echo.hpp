#ifndef PROTOBUF_ECHO_HPP
#define PROTOBUF_ECHO_HPP

#include "infra/util/BoundedDeque.hpp"
#include "infra/util/Compatibility.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include "protobuf/echo/Proto.hpp"
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

    class Service
        : public infra::Observer<Service, Echo>
    {
    public:
        using infra::Observer<Service, Echo>::Observer;

        virtual bool AcceptsService(uint32_t id) const = 0;

        void MethodDone();
        bool InProgress() const;
        void HandleMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy);

    protected:
        Echo& Rpc();
        virtual void Handle(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, EchoErrorPolicy& errorPolicy) = 0;

    private:
        bool inProgress = false;
    };

    class Echo
        : public infra::Subject<Service>
    {
    public:
        virtual void RequestSend(ServiceProxy& serviceProxy) = 0;
        virtual infra::StreamWriter& SendStreamWriter() = 0;
        virtual void Send() = 0;
        virtual void ServiceDone(Service& service) = 0;
    };

    class ServiceProxy
        : public infra::IntrusiveList<ServiceProxy>::NodeType
    {
    public:
        ServiceProxy(Echo& echo, uint32_t maxMessageSize);

        Echo& Rpc();
        virtual void RequestSend(infra::Function<void()> onGranted);
        virtual void RequestSend(infra::Function<void()> onGranted, uint32_t requestedSize);
        void GrantSend();
        uint32_t MaxMessageSize() const;
        uint32_t CurrentRequestedSize() const;

    private:
        Echo& echo;
        uint32_t maxMessageSize;
        infra::Function<void()> onGranted;
        uint32_t currentRequestedSize = 0;
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

        // Implementation of Echo
        void RequestSend(ServiceProxy& serviceProxy) override;
        infra::StreamWriter& SendStreamWriter() override;
        void Send() override;
        void ServiceDone(Service& service) override;

    protected:
        virtual void RequestSendStream(std::size_t size) = 0;
        virtual void BusyServiceDone() = 0;

        virtual void ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents, infra::StreamReaderWithRewinding& reader);
        virtual void SetStreamWriter(infra::SharedPtr<infra::StreamWriter>&& writer);
        bool ServiceBusy() const;
        bool ProcessMessage(infra::StreamReaderWithRewinding& reader);

    protected:
        EchoErrorPolicy& errorPolicy;

    private:
        infra::SharedPtr<infra::StreamWriter> streamWriter;
        infra::IntrusiveList<ServiceProxy> sendRequesters;
        infra::Optional<uint32_t> serviceBusy;
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
}

#endif
