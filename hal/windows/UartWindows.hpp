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

    class UartWindows
        : public SerialCommunication
    {
    public:
        struct DeviceName {};
        static const DeviceName deviceName;

        UartWindows(const std::string& portName);
        UartWindows(const std::string& name, DeviceName);
        ~UartWindows();

        virtual void ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived) override;
        virtual void SendData(infra::MemoryRange<const uint8_t> data, infra::Function<void()> actionOnCompletion = infra::emptyFunction) override;

    private:
        void Open(const std::string& name);
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

    class UartPortFinder
    {
    public:
        UartPortFinder();

        class UartNotFound
            : public std::runtime_error
        {
        public:
            UartNotFound(const std::string& portName);
        };

        std::string PhysicalDeviceObjectNameForDeviceDescription(const std::string& deviceDescription) const;
        std::string PhysicalDeviceObjectNameForMatchingDeviceId(const std::string& matchingDeviceId) const;

    private:
        void ReadAllComPortDevices();
        void ReadDevice(SP_DEVINFO_DATA& deviceInfo);

    private:
        struct Description
        {
            std::string deviceDescription;
            std::string friendlyName;
            std::string physicalDeviceObjectName;
            std::string matchingDeviceId;
        };

        std::vector<Description> descriptions;

        HDEVINFO deviceInformationSet;
    };
}

#endif
