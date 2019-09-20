#ifndef HAL_UART_WINDOWS_HPP
#define HAL_UART_WINDOWS_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include <atomic>
#include <mutex>
#include <thread>

namespace hal
{
    class UartWindows
        : public SerialCommunication
    {
    public:
        UartWindows(const std::string& portName);
        ~UartWindows();

        virtual void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;
        virtual void SendData(infra::MemoryRange<const uint8_t> data, infra::Function<void()> actionOnCompletion = infra::emptyFunction) override;

    private:
        void ReadThread();

        infra::Function<void(infra::ConstByteRange data)> onReceivedData;        

        void* handle = nullptr;
        std::atomic<bool> running = true;
        std::mutex mutex;
        std::condition_variable receivedDataSet;
        std::thread readThread;
    };
}

#endif
