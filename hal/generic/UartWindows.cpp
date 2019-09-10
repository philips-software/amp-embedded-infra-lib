#include "hal/generic/UartWindows.hpp"

#include <windows.h>

namespace hal
{  
    UartWindows::UartWindows(std::string portName)
    : readThread([this]() { ReadThread(); })
    {
        std::string port = "\\\\.\\" + portName;
        handle = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

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
        commTimeOuts.ReadIntervalTimeout = 3;
        commTimeOuts.ReadTotalTimeoutMultiplier = 3;
        commTimeOuts.ReadTotalTimeoutConstant = 2;
        commTimeOuts.WriteTotalTimeoutMultiplier = 0;
        commTimeOuts.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(handle, &commTimeOuts);
    }

    UartWindows::~UartWindows()
    {
        stop = true;
        readThread.join();
        CloseHandle(handle);
    }

    void UartWindows::ReadThread()
    {
        uint8_t buffer[1];
        unsigned long bytesRead;

        while (!stop)
        {
            if (executeRead)
            {
                ReadFile(handle, buffer, sizeof(buffer), &bytesRead, nullptr);
                if (bytesRead)
                    onReceivedData(infra::ConstByteRange(buffer, buffer+bytesRead));
            }
        }
    }

    void UartWindows::ReceiveData(infra::Function<void(infra::ConstByteRange data)> dataReceived)
    {
        onReceivedData = dataReceived;
        executeRead = true;
    }

    void UartWindows::SendData(infra::MemoryRange<const uint8_t> data, infra::Function<void()> actionOnCompletion)
    {
        unsigned long unused;
        WriteFile(handle, data.begin(), data.size(), &unused, nullptr);
        actionOnCompletion();
    }
}