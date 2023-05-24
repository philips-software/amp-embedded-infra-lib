#ifndef HAL_UART_UNIX_HPP
#define HAL_UART_UNIX_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace hal
{
    class UartUnix
        : public hal::SerialCommunication
    {
    public:
        explicit UartUnix(const std::string& portName);
        virtual ~UartUnix();

        // Implementation of hal::SerialCommunication
        void SendData(infra::ConstByteRange data, infra::Function<void()> actionOnCompletion) override;
        void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;

    private:
        void Open(const std::string& portName);
        void Read();

    private:
        int fileDescriptor{ -1 };
        uint8_t buffer;
        std::atomic<bool> running{ true };
        std::mutex mutex;
        std::condition_variable receivedDataSet;
        std::thread readThread;
        infra::Function<void(infra::ConstByteRange data)> receivedData;
    };
}

#endif
