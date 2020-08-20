#ifndef HAL_UART_WINDOWS_HPP
#define HAL_UART_WINDOWS_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include <atomic>
#include <mutex>
#include <thread>
#include <winsock2.h>
#include <windows.h>

namespace hal
{
    class PortNotOpened
        : public std::runtime_error
    {
    public:
        PortNotOpened(const std::string& portName);
    };

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
        void ReadNonBlocking(infra::ByteRange range);
        void ReadBlocking(infra::ByteRange range);

    private:
        infra::Function<void(infra::ConstByteRange data)> onReceivedData;

        void* handle = nullptr;
        std::atomic<bool> running = true;
        std::mutex mutex;
        std::condition_variable receivedDataSet;
        std::thread readThread;

        bool pendingRead = false;
        OVERLAPPED overlapped = { 0 };
        uint8_t buffer;
    };
}

#endif
