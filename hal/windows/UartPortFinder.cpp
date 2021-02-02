#include "hal/windows/UartPortFinder.hpp"
#include "infra/util/Optional.hpp"

#include <initguid.h>
#include <devpkey.h>

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
