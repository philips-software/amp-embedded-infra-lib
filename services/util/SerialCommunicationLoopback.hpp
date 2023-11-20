#ifndef SERVICES_SERIAL_COMMUNICATION_LOOPBACK_HPP
#define SERVICES_SERIAL_COMMUNICATION_LOOPBACK_HPP

#include "hal/interfaces/SerialCommunication.hpp"

namespace services
{

    class SerialCommunicationLoopback
    {
    public:
        SerialCommunicationLoopback();

        hal::SerialCommunication& Server();
        hal::SerialCommunication& Client();

    private:
        class SerialCommunicationLoopbackPeer
            : public hal::SerialCommunication
        {
        public:
            // pointer instead of reference to avoid defining a copy constructor
            // pass by reference generates a warning with clang and -Wunitialized
            explicit SerialCommunicationLoopbackPeer(SerialCommunicationLoopbackPeer* other);

            void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
            void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

        private:
            SerialCommunicationLoopbackPeer& other;

            infra::ConstByteRange data;
            infra::Function<void()> actionOnCompletion;
            infra::Function<void(infra::ConstByteRange data)> dataReceived;
        };

    private:
        SerialCommunicationLoopbackPeer server;
        SerialCommunicationLoopbackPeer client;
    };
}

#endif
