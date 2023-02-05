#include "hal/windows/UartWindows.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace hal
{
    const UartWindows::DeviceName UartWindows::deviceName;

    PortNotOpened::PortNotOpened(const std::string& portName)
        : std::runtime_error("Unable to open port " + portName)
    {}

    PortNotInitialized::PortNotInitialized(const std::string& portName, DWORD errorCode)
        : std::runtime_error("Unable to initialize port " + portName + " Errorcode: " + std::to_string(errorCode))
    {}

    UartWindows::UartWindows(const std::string& portName, UartWindowsConfig config)
    {
        Open(R"(\\.\)" + portName, config);
    }

    UartWindows::UartWindows(const std::string& name, DeviceName, UartWindowsConfig config)
    {
        Open(R"(\\.\GLOBALROOT)" + name, config);
    }

    UartWindows::~UartWindows()
    {
        {
            std::unique_lock<std::mutex> lock(mutex);
            running = false;
            receivedDataSet.notify_all();
        }

        if (readThread.joinable())
            readThread.join();
        CloseHandle(handle);
    }

    void UartWindows::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        std::unique_lock<std::mutex> lock(mutex);
        onReceivedData = dataReceived;
        receivedDataSet.notify_all();
    }

    void UartWindows::SendData(infra::MemoryRange<const uint8_t> data, infra::Function<void()> actionOnCompletion)
    {
        OVERLAPPED overlapped = { 0 };
        unsigned long bytesWritten;

        overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);

        if (!WriteFile(handle, data.begin(), data.size(), &bytesWritten, &overlapped))
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                if (WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0)
                {
                    if (!GetOverlappedResult(handle, &overlapped, &bytesWritten, false))
                        std::abort();
                }
            }
        }

        CloseHandle(overlapped.hEvent);

        infra::EventDispatcher::Instance().Schedule(actionOnCompletion);
    }

    void UartWindows::Open(const std::string& name, UartWindowsConfig config)
    {
        handle = CreateFile(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

        if (handle == INVALID_HANDLE_VALUE)
            throw PortNotOpened(name);

        DCB dcb;
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);
        dcb.BaudRate = config.baudRate;
        dcb.ByteSize = 8;
        dcb.Parity = static_cast<BYTE>(config.parity);
        dcb.StopBits = ONESTOPBIT;
        dcb.fRtsControl = (uint32_t)config.flowControl;
        dcb.fOutxCtsFlow = config.flowControl == UartWindowsConfig::RtsFlowControl::RtsHandshake;
        if (!SetCommState(handle, &dcb))
        {
            DWORD dw = GetLastError();
            throw PortNotInitialized(name, dw);
        }

        PurgeComm(handle, PURGE_TXCLEAR | PURGE_RXCLEAR);

        COMMTIMEOUTS commTimeOuts;
        commTimeOuts.ReadIntervalTimeout = MAXDWORD;
        commTimeOuts.ReadTotalTimeoutMultiplier = MAXDWORD;
        commTimeOuts.ReadTotalTimeoutConstant = 25;
        commTimeOuts.WriteTotalTimeoutMultiplier = 0;
        commTimeOuts.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(handle, &commTimeOuts);

        readThread = std::thread([this]()
            { ReadThread(); });
        SetThreadPriority(readThread.native_handle(), GetThreadPriority(reinterpret_cast<HANDLE>(readThread.native_handle())) + 1);
    }

    void UartWindows::ReadThread()
    {
        overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);

        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!onReceivedData)
                receivedDataSet.wait(lock);
        }

        while (running)
        {
            if (!pendingRead)
                ReadNonBlocking(infra::MakeByteRange(buffer));

            if (pendingRead)
                ReadBlocking(infra::MakeByteRange(buffer));
        }

        CloseHandle(overlapped.hEvent);
    }

    void UartWindows::ReadNonBlocking(infra::ByteRange range)
    {
        unsigned long bytesRead;

        if (!ReadFile(handle, range.begin(), range.size(), &bytesRead, &overlapped))
        {
            if (GetLastError() == ERROR_IO_PENDING)
                pendingRead = true;
        }
        else
        {
            if (bytesRead != 0)
            {
                std::unique_lock<std::mutex> lock(mutex);
                onReceivedData(infra::ConstByteRange(range.begin(), range.begin() + bytesRead));
            }
        }
    }

    void UartWindows::ReadBlocking(infra::ByteRange range)
    {
        const uint16_t readTimeoutMs = 500;
        if (WaitForSingleObject(overlapped.hEvent, readTimeoutMs) == WAIT_OBJECT_0)
        {
            unsigned long bytesRead;
            if (!GetOverlappedResult(handle, &overlapped, &bytesRead, false))
                std::abort();

            if (bytesRead != 0)
            {
                std::unique_lock<std::mutex> lock(mutex);
                onReceivedData(infra::ConstByteRange(range.begin(), range.begin() + bytesRead));
            }

            pendingRead = false;
        }
    }
}
