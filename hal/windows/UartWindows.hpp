#ifndef HAL_UART_WINDOWS_HPP
#define HAL_UART_WINDOWS_HPP

#include "hal/interfaces/SerialCommunication.hpp"
#include <atomic>
#include <mutex>
#include <thread>
#include <winsock2.h>
#include <windows.h>
#include <setupapi.h>

namespace hal
{
    class PortNotOpened
        : public std::runtime_error
    {
    public:
        PortNotOpened(const std::string& portName);
    };

    class PortNotInitialized
        : public std::runtime_error
    {
    public:
        PortNotInitialized(const std::string& portName, DWORD errorCode);
    };

    class UartWindows
        : public SerialCommunication
    {
    public:
        struct UartWindowsConfig
        {
            enum class RtsFlowControl
            {
                RtsControlDisable = RTS_CONTROL_DISABLE,
                RtsControlEnable = RTS_CONTROL_ENABLE,
                RtsHandshake = RTS_CONTROL_HANDSHAKE,
                RtsControlToggle = RTS_CONTROL_TOGGLE
            };

            enum class Parity
            {
                none = NOPARITY,
                even = EVENPARITY,
                odd = ODDPARITY,
                mark = MARKPARITY,
                space = SPACEPARITY
            };

            UartWindowsConfig() = default;

            UartWindowsConfig(uint32_t baudRate, RtsFlowControl flowControl)
                : baudRate(baudRate)
                , flowControl(flowControl)
            {}

            uint32_t baudRate = CBR_115200;
            RtsFlowControl flowControl = RtsFlowControl::RtsControlDisable;
            Parity parity = Parity::none;
        };

        struct DeviceName
        {};

        static const DeviceName deviceName;

        UartWindows(const std::string& portName, UartWindowsConfig config = UartWindowsConfig());
        UartWindows(const std::string& name, DeviceName, UartWindowsConfig config = UartWindowsConfig());
        virtual ~UartWindows();

        virtual void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;
        virtual void SendData(infra::MemoryRange<const uint8_t> data, infra::Function<void()> actionOnCompletion = infra::emptyFunction) override;

    private:
        void Open(const std::string& name, UartWindowsConfig config = UartWindowsConfig());
        void ReadThread();
        void ReadNonBlocking(infra::ByteRange range);
        void ReadBlocking(infra::ByteRange range);

    private:
        infra::Function<void(infra::ConstByteRange data)> onReceivedData;

        void* handle = nullptr;
        std::atomic<bool> running{ true };
        std::mutex mutex;
        std::condition_variable receivedDataSet;
        std::thread readThread;

        bool pendingRead = false;
        OVERLAPPED overlapped = { 0 };
        uint8_t buffer;
    };
}

#endif
