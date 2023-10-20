#include "services/util/SerialCommunicationLoopback.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    SerialCommunicationLoopbackPeer::SerialCommunicationLoopbackPeer(SerialCommunicationLoopbackPeer* other)
        : other{ *other }
    {}

    void SerialCommunicationLoopbackPeer::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        this->data = data;
        this->actionOnCompletion = actionOnCompletion;

        infra::EventDispatcher::Instance().Schedule([this]
            {
                other.dataReceived(this->data);
                this->actionOnCompletion();
            });
    }

    void SerialCommunicationLoopbackPeer::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        this->dataReceived = dataReceived;
    }

    SerialCommunicationLoopback::SerialCommunicationLoopback()
        : server(&client)
        , client(&server)
    {}

    SerialCommunicationLoopbackPeer& SerialCommunicationLoopback::Server()
    {
        return server;
    }

    SerialCommunicationLoopbackPeer& SerialCommunicationLoopback::Client()
    {
        return client;
    }
}
