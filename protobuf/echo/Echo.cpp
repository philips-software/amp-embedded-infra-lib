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

    uint32_t Service::ServiceId() const
    {
        return serviceId;
    }

    bool Service::InProgress() const
    {
        return inProgress;
    }

    void Service::HandleMethod(uint32_t methodId, infra::ProtoParser& parser)
    {
        inProgress = true;
        Handle(methodId, parser);
    }

    Echo& Service::Rpc()
    {
        return echo;
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

    void EchoOnConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        streamWriter = std::move(writer);

        ServiceProxy& proxy = sendRequesters.front();
        sendRequesters.pop_front();
        proxy.GrantSend();
    }

    void EchoOnConnection::DataReceived()
    {
        while (!serviceBusy)
        {
            infra::SharedPtr<infra::StreamReader> reader = ConnectionObserver::Subject().ReceiveStream();
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

            ConnectionObserver::Subject().AckReceived();

            if (stream.Empty())
                break;
        }
    }

    void EchoOnConnection::RequestSend(ServiceProxy& serviceProxy)
    {
        if (sendRequesters.empty() && streamWriter == nullptr)
            ConnectionObserver::Subject().RequestSendStream(serviceProxy.MaxMessageSize());

        sendRequesters.push_back(serviceProxy);
    }

    infra::StreamWriter& EchoOnConnection::SendStreamWriter()
    {
        return *streamWriter;
    }

    void EchoOnConnection::Send()
    {
        streamWriter = nullptr;

        if (!sendRequesters.empty())
            ConnectionObserver::Subject().RequestSendStream(sendRequesters.front().MaxMessageSize());
    }

    void EchoOnConnection::ServiceDone(Service& service)
    {
        if (serviceBusy && *serviceBusy == service.ServiceId())
        {
            serviceBusy = infra::none;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](infra::SharedPtr<EchoOnConnection> echo) { echo->DataReceived(); }, SharedFromThis());
        }
    }

    void EchoOnConnection::AttachService(Service& service)
    {
        services.push_back(service);
    }

    void EchoOnConnection::DetachService(Service& service)
    {
        services.erase(service);
    }

    void EchoOnConnection::ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoParser& argument)
    {
        for (auto& service : services)
            if (service.ServiceId() == serviceId)
            {
                if (service.InProgress())
                    serviceBusy = serviceId;
                else
                    service.HandleMethod(methodId, argument);
                break;
            }
    }
}
