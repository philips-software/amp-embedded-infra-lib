#include "services/util/SerialCommunicationLoopback.hpp"
#include "hal/interfaces/SerialCommunication.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    SerialCommunicationLoopback::SerialCommunicationLoopback()
        : server(&client)
        , client(&server)
    {}

    hal::SerialCommunication& SerialCommunicationLoopback::Server()
    {
        return server;
    }

    hal::SerialCommunication& SerialCommunicationLoopback::Client()
    {
        return client;
    }

    SerialCommunicationLoopback::SerialCommunicationLoopbackPeer::SerialCommunicationLoopbackPeer(SerialCommunicationLoopbackPeer* other)
        : other{ *other }
    {}

    void SerialCommunicationLoopback::SerialCommunicationLoopbackPeer::SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion)
    {
        this->data = data;
        this->actionOnCompletion = actionOnCompletion;

        infra::EventDispatcher::Instance().Schedule([this]
            {
                other.dataReceived(this->data);
                this->actionOnCompletion();
            });
    }

    void SerialCommunicationLoopback::SerialCommunicationLoopbackPeer::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        this->dataReceived = dataReceived;
    }
}
