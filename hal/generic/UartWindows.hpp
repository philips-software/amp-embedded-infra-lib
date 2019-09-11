#ifndef HAL_UART_WINDOWS_HPP
#define HAL_UART_WINDOWS_HPP

#include "hal/interfaces/SerialCommunication.hpp"

#include <thread>

namespace hal
{
    class UartWindows
        : public SerialCommunication
    {
    public:
        UartWindows(std::string portName);
        ~UartWindows();

        virtual void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;
        virtual void SendData(infra::MemoryRange<const uint8_t> data, infra::Function<void()> actionOnCompletion = infra::emptyFunction) override;

    private:
        void ReadThread();

        infra::Function<void(infra::ConstByteRange data)> onReceivedData;        
        
        volatile bool executeRead = false;
        volatile bool stop = false;
        void* handle;

        std::thread readThread;
    };
}

#endif
