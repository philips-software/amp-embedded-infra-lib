#ifndef SERVICES_SERIAL_COMMUNICATION_LOOPBACK_HPP
#define SERVICES_SERIAL_COMMUNICATION_LOOPBACK_HPP

#include "hal/interfaces/SerialCommunication.hpp"

namespace services
{
    class SerialCommunicationLoopbackPeer
        : public hal::SerialCommunication
    {
    public:
        explicit SerialCommunicationLoopbackPeer(SerialCommunicationLoopbackPeer* other);

        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

    private:
        SerialCommunicationLoopbackPeer& other;

        infra::ConstByteRange data;
        infra::Function<void()> actionOnCompletion;
        infra::Function<void(infra::ConstByteRange data)> dataReceived;
    };

    class SerialCommunicationLoopback
    {
    public:
        SerialCommunicationLoopback();

        SerialCommunicationLoopbackPeer& Server();
        SerialCommunicationLoopbackPeer& Client();

    private:
        SerialCommunicationLoopbackPeer server;
        SerialCommunicationLoopbackPeer client;
    };
}

#endif
