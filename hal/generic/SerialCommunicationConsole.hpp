#ifndef HAL_SERIAL_COMMUNICATION_CONSOLE_HPP
#define HAL_SERIAL_COMMUNICATION_CONSOLE_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include <iostream>
#include <thread>

namespace hal
{
    class SerialCommunicationConsole
        : public SerialCommunication
    {
    public:
        SerialCommunicationConsole();

        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

    private:
        infra::Function<void(infra::ConstByteRange data)> dataReceived;
    };
}

#endif
