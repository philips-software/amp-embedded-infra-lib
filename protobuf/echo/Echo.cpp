#include "infra/event/EventDispatcherWithWeakPtr.hpp"
#include "infra/syntax/ProtoFormatter.hpp"
#include "protobuf/echo/Echo.hpp"

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

    void Service::HandleMethod(uint32_t methodId, infra::ProtoLengthDelimited& contents)
    {
        inProgress = true;
        Handle(methodId, contents);
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

    ServiceForwarder::ServiceForwarder(infra::ByteRange messageBuffer, Echo& echo, uint32_t id, Echo& forwardTo)
        : Service(echo, id)
        , ServiceProxy(forwardTo, id, messageBuffer.size())
        , messageBuffer(messageBuffer)
    {}

    void ServiceForwarder::Handle(uint32_t methodId, infra::ProtoLengthDelimited& contents)
    {
        this->methodId = methodId;

        bytes.Emplace(messageBuffer);
        uint32_t processedSize = 0;

        while (true)
        {
            infra::ConstByteRange contiguousBytes;
            contents.GetBytesReference(contiguousBytes);

            if (contiguousBytes.size() == 0)
                break;

            std::copy(contiguousBytes.begin(), contiguousBytes.end(), bytes->begin() + processedSize);
            processedSize += contiguousBytes.size();
        }

        bytes->shrink_from_back_to(processedSize);

        RequestSend([this, &contents]()
        {
            infra::DataOutputStream::WithErrorPolicy stream(services::ServiceProxy::Rpc().SendStreamWriter());
            infra::ProtoFormatter formatter(stream);
            formatter.PutVarInt(ServiceId());            
            formatter.PutBytesField(*bytes, this->methodId);
            bytes = infra::none;

            services::ServiceProxy::Rpc().Send();
            MethodDone();
        });
    }

    void EchoOnStreams::RequestSend(ServiceProxy& serviceProxy)
    {
        if (sendRequesters.empty() && streamWriter == nullptr)
        {
            sendRequesters.push_back(serviceProxy);
            RequestSendStream(serviceProxy.MaxMessageSize());
        }
        else
            sendRequesters.push_back(serviceProxy);
    }

    infra::StreamWriter& EchoOnStreams::SendStreamWriter()
    {
        return *streamWriter;
    }

    void EchoOnStreams::Send()
    {
        streamWriter = nullptr;

        if (!sendRequesters.empty())
            RequestSendStream(sendRequesters.front().MaxMessageSize());
    }

    void EchoOnStreams::ServiceDone(Service& service)
    {
        if (serviceBusy && *serviceBusy == service.ServiceId())
        {
            serviceBusy = infra::none;
            infra::EventDispatcherWithWeakPtr::Instance().Schedule([](infra::SharedPtr<EchoOnStreams> echo) { echo->BusyServiceDone(); }, SharedFromThis());
        }
    }

    void EchoOnStreams::AttachService(Service& service)
    {
    	for (auto& s : services)
    		if (s.ServiceId() == service.ServiceId())
    			std::abort();

        services.push_back(service);
    }

    void EchoOnStreams::DetachService(Service& service)
    {
        services.erase(service);
    }

    void EchoOnStreams::ExecuteMethod(uint32_t serviceId, uint32_t methodId, infra::ProtoLengthDelimited& contents)
    {
        for (auto& service : services)
            if (service.ServiceId() == serviceId)
            {
                if (service.InProgress())
                    serviceBusy = serviceId;
                else
                    service.HandleMethod(methodId, contents);
                break;
            }
    }

    void EchoOnStreams::SetStreamWriter(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        streamWriter = std::move(writer);

        ServiceProxy& proxy = sendRequesters.front();
        sendRequesters.pop_front();
        proxy.GrantSend();
    }

    bool EchoOnStreams::ServiceBusy() const
    {
        return serviceBusy != infra::none;
    }

    void EchoOnConnection::SendStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        SetStreamWriter(std::move(writer));
    }

    void EchoOnConnection::DataReceived()
    {
        while (!ServiceBusy())
        {
            infra::SharedPtr<infra::StreamReader> reader = ConnectionObserver::Subject().ReceiveStream();
            infra::DataInputStream::WithErrorPolicy stream(*reader, infra::softFail);
            infra::ProtoParser parser(stream);
            uint32_t serviceId = static_cast<uint32_t>(parser.GetVarInt());
            infra::ProtoParser::Field message = parser.GetField();
            if (stream.Failed())
                break;

            ExecuteMethod(serviceId, message.second, message.first.Get<infra::ProtoLengthDelimited>());
            if (stream.Failed())
                std::abort();
            if (ServiceBusy())
                break;

            ConnectionObserver::Subject().AckReceived();

            if (stream.Empty())
                break;
        }
    }

    void EchoOnConnection::RequestSendStream(std::size_t size)
    {
        ConnectionObserver::Subject().RequestSendStream(size);
    }

    void EchoOnConnection::BusyServiceDone()
    {
        DataReceived();
    }

    void EchoOnMessageCommunication::SendMessageStreamAvailable(infra::SharedPtr<infra::StreamWriter>&& writer)
    {
        SetStreamWriter(std::move(writer));
    }

    void EchoOnMessageCommunication::ReceivedMessage(infra::SharedPtr<infra::StreamReaderWithRewinding>&& reader)
    {
        this->reader = std::move(reader);
        ProcessMessage();
    }

    void EchoOnMessageCommunication::ServiceDone(Service& service)
    {
        reader = nullptr;
        EchoOnStreams::ServiceDone(service);
    }

    void EchoOnMessageCommunication::RequestSendStream(std::size_t size)
    {
        MessageCommunicationObserver::Subject().RequestSendMessage(static_cast<uint16_t>(size));
    }

    void EchoOnMessageCommunication::BusyServiceDone()
    {
        // In this class, services are never busy, so BusyServiceDone() is never invoked
        std::abort();
    }

    void EchoOnMessageCommunication::ProcessMessage()
    {
        infra::DataInputStream::WithErrorPolicy stream(*reader, infra::softFail);
        infra::ProtoParser parser(stream);
        uint32_t serviceId = static_cast<uint32_t>(parser.GetVarInt());
        infra::ProtoParser::Field message = parser.GetField();
        if (stream.Failed())
            return;

        assert(message.first.Is<infra::ProtoLengthDelimited>());
        ExecuteMethod(serviceId, message.second, message.first.Get<infra::ProtoLengthDelimited>());
        if (stream.Failed())
            std::abort();
    }
}
