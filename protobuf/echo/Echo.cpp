#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "protobuf/echo/Echo.hpp"
#include "infra/syntax/ProtoFormatter.hpp"

namespace services
{
    Service::Service(Echo& echo, uint32_t id)
        : echo(echo)
        , serviceId(id)
    {
        echo.AttachService(*this);
    }

    Service::~Service()
    {
        echo.DetachService(*this);
    }

    void Service::MethodDone()
    {
        inProgress = false;
        Rpc().ServiceDone(*this);
    }

    Echo& Service::Rpc()
    {
        return echo;
    }

    uint32_t Service::ServiceId() const
    {
        return serviceId;
    }

    ServiceProxy::ServiceProxy(Echo& echo, uint32_t id, uint32_t maxMessageSize)
        : echo(echo)
        , serviceId(id)
        , maxMessageSize(maxMessageSize)
    {}

    Echo& ServiceProxy::Rpc()
    {
        return echo;
    }

    void ServiceProxy::RequestSend(infra::Function<void()> onGranted)
    {
        this->onGranted = onGranted;
        echo.RequestSend(*this);
    }

    void ServiceProxy::GrantSend()
    {
        onGranted();
    }

    uint32_t ServiceProxy::MaxMessageSize() const
    {
        return maxMessageSize;
    }

    void Echo::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        streamWriter = std::move(writer);

        ServiceProxy& proxy = sendRequesters.front();
        sendRequesters.pop_front();
        proxy.GrantSend();
    }

    void Echo::DataReceived()
    {
        while (!serviceBusy)
        {
            infra::SharedPtr<infra::StreamReader> reader = services::ConnectionObserver::Subject().ReceiveStream();
            infra::DataInputStream::WithErrorPolicy stream(*reader, infra::softFail);
            infra::ProtoParser parser(stream);
            uint32_t serviceId = static_cast<uint32_t>(parser.GetVarInt());
            infra::ProtoParser::Field message = parser.GetField();
            if (stream.Failed())
                break;

            assert(message.first.Is<infra::ProtoLengthDelimited>());
            infra::ProtoParser argument(message.first.Get<infra::ProtoLengthDelimited>().Parser());
            uint32_t methodId = message.second;
            if (stream.Failed())
                break;

            ExecuteMethod(serviceId, methodId, argument);
            if (stream.Failed())
                std::abort();
            if (serviceBusy)
                break;

            services::ConnectionObserver::Subject().AckReceived();

            if (stream.Empty())
                break;
        }
    }

    void Echo::RequestSend(ServiceProxy& serviceProxy)
    {
        if (sendRequesters.empty() && streamWriter == nullptr)
            services::ConnectionObserver::Subject().RequestSendStream(serviceProxy.MaxMessageSize());

        sendRequesters.push_back(serviceProxy);
    }

    infra::StreamWriter& Echo::SendStreamWriter()
    {
        return *streamWriter;
    }

    void Echo::Send()
    {
        streamWriter = nullptr;

        if (!sendRequesters.empty())
            services::ConnectionObserver::Subject().RequestSendStream(sendRequesters.front().MaxMessageSize());
    }

    void Echo::ServiceDone(Service& service)
    {
        if (serviceBusy && *serviceBusy == service.ServiceId())
        {
            serviceBusy = infra::none;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](infra::SharedPtr<Echo> echo) { echo->DataReceived(); }, SharedFromThis());
        }
    }

    void Echo::AttachService(Service& service)
    {
        services.push_back(service);
    }

    void Echo::DetachService(Service& service)
    {
        services.erase(service);
    }

    void Echo::ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoParser& argument)
    {
        for (auto& service : services)
        {
            if (service.ServiceId() == serviceId)
            {
                if (service.inProgress)
                    serviceBusy = serviceId;
                else
                {
                    service.inProgress = true;
                    service.Handle(methodId, argument);
                }
                break;
            }
        };
    }
}
