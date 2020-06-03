#include "hal/windows/UartWindows.hpp"
#include <windows.h>

namespace hal
{
    UartWindows::UartWindows(const std::string& portName)
    {
        std::string port = "\\\\.\\" + portName;
        handle = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

        if (handle == INVALID_HANDLE_VALUE)
            return;

        DCB dcb;
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);
        dcb.BaudRate = CBR_115200;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;
        SetCommState(handle, &dcb);
        PurgeComm(handle, PURGE_TXCLEAR | PURGE_RXCLEAR);

        COMMTIMEOUTS commTimeOuts;
        commTimeOuts.ReadIntervalTimeout = MAXDWORD;
        commTimeOuts.ReadTotalTimeoutMultiplier = MAXDWORD;
        commTimeOuts.ReadTotalTimeoutConstant = 25;
        commTimeOuts.WriteTotalTimeoutMultiplier = 0;
        commTimeOuts.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(handle, &commTimeOuts);

        readThread = std::thread([this]() { ReadThread(); });
    }

    UartWindows::~UartWindows()
    {
        running = false;
        if (readThread.joinable())
            readThread.join();
        CloseHandle(handle);
    }

    void UartWindows::ReadThread()
    {
        const uint8_t readTimeoutMs = 500;

        unsigned long bytesRead;
        bool pendingRead = false;
        OVERLAPPED overlapped = { 0 };
        uint8_t buffer[1];

        overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);

        std::unique_lock<std::mutex> lock(mutex);
        if (!onReceivedData)
            receivedDataSet.wait(lock);

        while (running)
        {
            if (!pendingRead) 
            {
                if (!ReadFile(handle, buffer, sizeof(buffer), &bytesRead, &overlapped))
                {
                    if (GetLastError() == ERROR_IO_PENDING)
                        pendingRead = true;
                }
                else 
                {
                    if (bytesRead)
                        onReceivedData(infra::ConstByteRange(buffer, buffer + bytesRead));
                }
            }

            if (pendingRead) 
            {
                if (WaitForSingleObject(overlapped.hEvent, readTimeoutMs) == WAIT_OBJECT_0)
                {
                    if (GetOverlappedResult(handle, &overlapped, &bytesRead, false))
                    {
                        if (bytesRead)
                            onReceivedData(infra::ConstByteRange(buffer, buffer + bytesRead));
                        pendingRead = false;
                    }
                    else
                        std::abort();
                }
            }
        }

        CloseHandle(overlapped.hEvent);
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
       
       actionOnCompletion();
    }
}
