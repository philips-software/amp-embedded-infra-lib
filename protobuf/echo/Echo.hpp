#ifndef PROTOBUF_ECHO_HPP
#define PROTOBUF_ECHO_HPP

#include "infra/util/Function.hpp"
#include "infra/util/Optional.hpp"
#include "infra/syntax/ProtoParser.hpp"
#include "services/network/Connection.hpp"

namespace services
{
    class Echo;

    class Service
        : public infra::IntrusiveList<Service>::NodeType
    {
    public:
        Service(Echo& echo, uint32_t id);
        Service(const Service& other) = delete;
        Service& operator=(const Service& other) = delete;
        ~Service();

        void MethodDone();

    protected:
        virtual void Handle(uint32_t methodId, infra::ProtoParser& parser) = 0;
        Echo& Rpc();

    private:
        friend class Echo;

        uint32_t ServiceId() const;

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

    class Echo
        : public services::ConnectionObserver
        , public infra::EnableSharedFromThis<Echo>
    {
    public:
        virtual void SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer) override;
        virtual void DataReceived() override;

        void RequestSend(ServiceProxy& serviceProxy);
        infra::StreamWriter& SendStreamWriter();
        void Send();
        void ServiceDone(Service& service);

        void AttachService(Service& service);
        void DetachService(Service& service);

    private:
        void ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoParser& argument);

    private:
        infra::IntrusiveList<Service> services;
        infra::SharedPtr<infra::StreamWriter> streamWriter;
        infra::IntrusiveList<ServiceProxy> sendRequesters;
        infra::Optional<uint32_t> serviceBusy;
    };
}

#endif
