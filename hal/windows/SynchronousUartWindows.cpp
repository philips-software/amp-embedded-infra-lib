#include "hal/windows/SynchronousUartWindows.hpp"

namespace hal
{
    SynchronousUartWindows::SynchronousUartWindows(const std::string& name, UartWindowsConfig config)
    {
        Open(R"(\\.\)" + name, config);
    }

    SynchronousUartWindows::~SynchronousUartWindows()
    {
        CloseHandle(handle);
    }

    void SynchronousUartWindows::SendData(infra::ConstByteRange data)
    {
        DWORD bytesWritten = 0;
        WriteFile(handle, data.begin(), data.size(), &bytesWritten, nullptr);
    }

    bool SynchronousUartWindows::ReceiveData(infra::ByteRange data)
    {
        DWORD bytesRead = 0;
        ReadFile(handle, data.begin(), data.size(), &bytesRead, nullptr);
        return data.size() == bytesRead;
    }

    void SynchronousUartWindows::Open(const std::string& name, UartWindowsConfig config)
    {
        handle = CreateFile(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

        if (handle == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Unable to open port " + name);

        DCB dcb;
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);
        dcb.BaudRate = config.baudRate;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;
        dcb.fRtsControl = (uint32_t)config.flowControlRts;
        dcb.fOutxCtsFlow = config.flowControlRts == UartWindowsConfig::RtsFlowControl::RtsHandshake;
        if (!SetCommState(handle, &dcb))
            throw std::runtime_error("Unable to initialize port " + name + " Errorcode: " + std::to_string(GetLastError()));

        PurgeComm(handle, PURGE_TXCLEAR | PURGE_RXCLEAR);

        COMMTIMEOUTS commTimeOuts;
        commTimeOuts.ReadIntervalTimeout = MAXDWORD;
        commTimeOuts.ReadTotalTimeoutMultiplier = 0;
        commTimeOuts.ReadTotalTimeoutConstant = 0;
        commTimeOuts.WriteTotalTimeoutMultiplier = 0;
        commTimeOuts.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(handle, &commTimeOuts);
    }
}
