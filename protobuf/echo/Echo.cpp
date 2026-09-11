#include "protobuf/echo/Echo.hpp"
#include "infra/util/ReallyAssert.hpp"

namespace services
{
    void Service::MethodDone()
    {
        Rpc().ServiceDone();
    }

    Service::Service(Echo& echo, EchoChannel channel)
        : infra::Observer<Service, Echo>(echo)
        , channel(channel)
    {}

    EchoChannel Service::Channel() const
    {
        return channel;
    }

    void Service::SetChannel(EchoChannel channel)
    {
        this->channel = channel;
    }

    Echo& Service::Rpc()
    {
        return Subject();
    }

    ServiceProxy::ServiceProxy(Echo& echo, uint32_t maxMessageSize, EchoChannel channel)
        : echo(echo)
        , maxMessageSize(maxMessageSize)
        , channel(channel)
    {}

    ServiceProxy::~ServiceProxy()
    {
        if (onGranted != nullptr)
            CancelRequestSend();
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
        really_assert_with_msg(!this->onGranted, "ServiceProxy::RequestSend onGranted already pending; proxy=%p pending=%p incoming=%p", static_cast<const void*>(this), this->onGranted.TargetType(), onGranted.TargetType());
        this->onGranted = onGranted;
        currentRequestedSize = requestedSize;
        echo.RequestSend(*this);
    }

    infra::SharedPtr<MethodSerializer> ServiceProxy::GrantSend()
    {
        onGranted();
        return std::move(methodSerializer);
    }

    void ServiceProxy::CancelRequestSend()
    {
        onGranted = nullptr;
        currentRequestedSize = 0;
        echo.CancelRequestSend(*this);
    }

    uint32_t ServiceProxy::MaxMessageSize() const
    {
        return maxMessageSize;
    }

    uint32_t ServiceProxy::CurrentRequestedSize() const
    {
        return currentRequestedSize;
    }

    EchoChannel ServiceProxy::Channel() const
    {
        return channel;
    }

    void ServiceProxy::SetChannel(EchoChannel channel)
    {
        this->channel = channel;
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
