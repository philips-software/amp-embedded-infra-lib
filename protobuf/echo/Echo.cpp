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

    void EchoPolicy::RequestSend(ServiceProxy& proxy, const infra::Function<void(ServiceProxy& proxy)>& onRequest)
    {
        onRequest(proxy);
    }

    void EchoPolicy::GrantingSend(ServiceProxy& proxy)
    {}
}
