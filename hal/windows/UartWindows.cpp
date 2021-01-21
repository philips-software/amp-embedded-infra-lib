#include "hal/windows/UartWindows.hpp"
#include <initguid.h>
#include <devpkey.h>
#include "services/tracer/GlobalTracer.hpp"

namespace hal
{
    namespace
    {
        std::string ConvertMbcsToUtf8(const std::wstring& from)
        {
            auto size = WideCharToMultiByte(CP_UTF8, 0, from.data(), from.size(), nullptr, 0, nullptr, nullptr);
            assert(size >= 0);

            std::string result(size, ' ');
            WideCharToMultiByte(CP_UTF8, 0, from.data(), from.size(), const_cast<char*>(result.data()), result.size(), nullptr, nullptr);

            return result;
        }

        std::wstring ConvertBufferToString(const std::vector<uint8_t>& buffer)
        {
            std::wstring result(reinterpret_cast<LPCWCH>(buffer.data()), reinterpret_cast<LPCWCH>(buffer.data()) + buffer.size() / 2);

            while (!result.empty() && result.back() == 0)
                result.pop_back();

            return result;
        }
    }

    const UartWindows::DeviceName UartWindows::deviceName;

    PortNotOpened::PortNotOpened(const std::string& portName)
        : std::runtime_error("Unable to open port " + portName)
    {}

    UartWindows::UartWindows(const std::string& portName)
    {
        Open(R"(\\.\)" + portName);
    }

    UartWindows::UartWindows(const std::string& name, DeviceName)
    {
        Open(R"(\\.\GLOBALROOT)" + name);
    }

    UartWindows::~UartWindows()
    {
        running = false;
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

    void UartWindows::Open(const std::string& name)
    {
        handle = CreateFile(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

        if (handle == INVALID_HANDLE_VALUE)
            throw PortNotOpened(name);

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
        SetThreadPriority(readThread.native_handle(), GetThreadPriority(readThread.native_handle()) + 1);
    }

    void UartWindows::ReadThread()
    {
        overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);

        std::unique_lock<std::mutex> lock(mutex);
        if (!onReceivedData)
            receivedDataSet.wait(lock);

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
                onReceivedData(infra::ConstByteRange(range.begin(), range.begin() + bytesRead));
        }
    }

    void UartWindows::ReadBlocking(infra::ByteRange range)
    {
        const uint16_t readTimeoutMs = 500;
        if (WaitForSingleObject(overlapped.hEvent, readTimeoutMs) == WAIT_OBJECT_0)
        {
            unsigned long bytesRead;
            if (GetOverlappedResult(handle, &overlapped, &bytesRead, false))
            {
                if (bytesRead != 0)
                    onReceivedData(infra::ConstByteRange(range.begin(), range.begin() + bytesRead));
                pendingRead = false;
            }
            else
                std::abort();
        }
    }

    UartPortFinder::UartPortFinder()
    {
        deviceInformationSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_COMPORT, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        assert(deviceInformationSet != INVALID_HANDLE_VALUE);

        ReadAllComPortDevices();

        SetupDiDestroyDeviceInfoList(deviceInformationSet);
    }

    std::string UartPortFinder::PhysicalDeviceObjectNameForDeviceDescription(const std::string& deviceDescription) const
    {
        for (auto& description : descriptions)
            if (description.deviceDescription == deviceDescription)
                return description.physicalDeviceObjectName;

        throw UartNotFound(deviceDescription);
    }

    std::string UartPortFinder::PhysicalDeviceObjectNameForMatchingDeviceId(const std::string& matchingDeviceId) const
    {
        for (auto& description : descriptions)
            if (description.matchingDeviceId == matchingDeviceId)
                return description.physicalDeviceObjectName;

        throw UartNotFound(matchingDeviceId);
    }

    void UartPortFinder::ReadAllComPortDevices()
    {
        SP_DEVINFO_DATA deviceInfo{};
        deviceInfo.cbSize = sizeof(SP_DEVINFO_DATA);

        for (int nIndex = 0; SetupDiEnumDeviceInfo(deviceInformationSet, nIndex, &deviceInfo); ++nIndex)
            ReadDevice(deviceInfo);
    }

    void UartPortFinder::ReadDevice(SP_DEVINFO_DATA& deviceInfo)
    {
        DWORD keysCount = 0;

        SetupDiGetDevicePropertyKeys(deviceInformationSet, &deviceInfo, nullptr, 0, &keysCount, 0);
        std::vector<DEVPROPKEY> keys(keysCount, DEVPROPKEY());
        auto result = SetupDiGetDevicePropertyKeys(deviceInformationSet, &deviceInfo, keys.data(), keys.size(), &keysCount, 0);
        assert(result == TRUE);

        infra::Optional<std::string> deviceDescription;
        infra::Optional<std::string> friendlyName;
        infra::Optional<std::string> physicalDeviceObjectName;
        infra::Optional<std::string> matchingDeviceId;

        for (auto& key : keys)
        {
            DEVPROPTYPE propType;

            DWORD bufferSize = 0;
            SetupDiGetDevicePropertyW(deviceInformationSet, &deviceInfo, &key, &propType, nullptr, 0, &bufferSize, 0);
            std::vector<uint8_t> buffer(bufferSize, 0);
            auto result = SetupDiGetDevicePropertyW(deviceInformationSet, &deviceInfo, &key, &propType, buffer.data(), buffer.size(), &bufferSize, 0);
            assert(result == TRUE);

            if (key == DEVPKEY_Device_DeviceDesc && propType == DEVPROP_TYPE_STRING)
                deviceDescription = ConvertMbcsToUtf8(ConvertBufferToString(buffer));
            if (key == DEVPKEY_Device_FriendlyName && propType == DEVPROP_TYPE_STRING)
                friendlyName = ConvertMbcsToUtf8(ConvertBufferToString(buffer));
            if (key == DEVPKEY_Device_PDOName && propType == DEVPROP_TYPE_STRING)
                physicalDeviceObjectName = ConvertMbcsToUtf8(ConvertBufferToString(buffer));
            if (key == DEVPKEY_Device_MatchingDeviceId && propType == DEVPROP_TYPE_STRING)
                matchingDeviceId = ConvertMbcsToUtf8(ConvertBufferToString(buffer));
        }

        if (deviceDescription && friendlyName && physicalDeviceObjectName)
            descriptions.push_back({ *deviceDescription, *friendlyName, *physicalDeviceObjectName, *matchingDeviceId });
    }

    UartPortFinder::UartNotFound::UartNotFound(const std::string& portName)
        : std::runtime_error("Unable to find a UART with name " + portName)
    {}
}
