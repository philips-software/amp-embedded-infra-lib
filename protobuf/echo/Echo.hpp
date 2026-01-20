#ifndef PROTOBUF_ECHO_HPP
#define PROTOBUF_ECHO_HPP

#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Observer.hpp"
#include "protobuf/echo/EchoErrorPolicy.hpp"
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
        ~ServiceProxy();

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
        infra::AutoResetFunction<void()> onGranted;
        uint32_t currentRequestedSize = 0;
        infra::SharedPtr<MethodSerializer> methodSerializer;
    };

    class EchoPolicy
    {
    public:
        EchoPolicy() = default;
        EchoPolicy(const EchoPolicy& other) = delete;
        EchoPolicy& operator=(const EchoPolicy& other) = delete;
        ~EchoPolicy() = default;

        virtual void RequestSend(ServiceProxy& proxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest);
        virtual void GrantingSend(ServiceProxy& proxy);
    };

    class Echo
        : public infra::Subject<Service>
    {
    public:
        virtual void RequestSend(ServiceProxy& serviceProxy) = 0;
        virtual void ServiceDone() = 0;
        virtual void CancelRequestSend(ServiceProxy& serviceProxy) = 0;
        virtual services::MethodSerializerFactory& SerializerFactory() = 0;
    };

    class EchoWithPolicy
        : public Echo
    {
    public:
        virtual void SetPolicy(EchoPolicy& policy) = 0;
    };

    class EchoInitialization;

    class EchoInitializationObserver
        : public infra::Observer<EchoInitializationObserver, EchoInitialization>
    {
    public:
        using infra::Observer<EchoInitializationObserver, EchoInitialization>::Observer;

        virtual void Initialized() = 0;
    };

    class EchoInitialization
        : public infra::Subject<EchoInitializationObserver>
    {};
}

#endif
