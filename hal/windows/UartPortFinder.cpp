#include "hal/windows/UartPortFinder.hpp"
#include "infra/util/Optional.hpp"
#include <devpkey.h>
#include <devpropdef.h>
#include <initguid.h>
#include <sstream>

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

        std::vector<std::string> ConvertMbcsToUtf8(const std::vector<std::wstring>& from)
        {
            std::vector<std::string> result;

            for (auto& i : from)
                result.push_back(ConvertMbcsToUtf8(i));

            return result;
        }

        std::wstring ConvertBufferToString(const std::vector<uint8_t>& buffer)
        {
            std::wstring result(reinterpret_cast<LPCWCH>(buffer.data()), reinterpret_cast<LPCWCH>(buffer.data()) + buffer.size() / 2);

            auto terminator = result.find(L'\0');
            if (terminator != std::wstring::npos)
                result.erase(terminator);

            return result;
        }

        std::vector<std::wstring> ConvertBufferToStringList(std::vector<uint8_t> buffer)
        {
            std::vector<std::wstring> result;

            auto item = ConvertBufferToString(buffer);
            while (!item.empty())
            {
                result.push_back(item);

                buffer.erase(buffer.begin(), buffer.begin() + 2 * item.size() + 2);

                item = ConvertBufferToString(buffer);
            }

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

    std::string UartPortFinder::PhysicalDeviceObjectNameForMatchingHardwareId(const std::string& matchingHardwareId) const
    {
        for (auto& description : descriptions)
            for (auto& hardwareId : description.hardwareIds)
                if (hardwareId == matchingHardwareId)
                    return description.physicalDeviceObjectName;

        throw UartNotFound(matchingHardwareId);
    }

    std::string UartPortFinder::ListAttributes() const
    {
        std::ostringstream stream;

        for (auto& description : descriptions)
        {
            stream
                << "==============" << std::endl
                << "deviceDescription: " << description.deviceDescription << std::endl
                << "friendlyName: " << description.friendlyName << std::endl
                << "physicalDeviceObjectName: " << description.physicalDeviceObjectName << std::endl
                << "matchingDeviceId: " << description.matchingDeviceId << std::endl;

            for (auto& hardwareId : description.hardwareIds)
                stream << "hardwareId: " << hardwareId << std::endl;
        }

        return stream.str();
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
        infra::Optional<std::vector<std::string>> hardwareIds;

        for (const DEVPROPKEY& key : keys)
        {
            DEVPROPTYPE propType;

            DWORD bufferSize = 0;
            SetupDiGetDevicePropertyW(deviceInformationSet, &deviceInfo, &key, &propType, nullptr, 0, &bufferSize, 0);
            std::vector<uint8_t> buffer(bufferSize, 0);
            auto result = SetupDiGetDevicePropertyW(deviceInformationSet, &deviceInfo, &key, &propType, buffer.data(), buffer.size(), &bufferSize, 0);
            assert(result == TRUE);

            if (IsEqualDevPropKey(key, DEVPKEY_Device_DeviceDesc) && propType == DEVPROP_TYPE_STRING)
                deviceDescription = ConvertMbcsToUtf8(ConvertBufferToString(buffer));
            if (IsEqualDevPropKey(key, DEVPKEY_Device_FriendlyName) && propType == DEVPROP_TYPE_STRING)
                friendlyName = ConvertMbcsToUtf8(ConvertBufferToString(buffer));
            if (IsEqualDevPropKey(key, DEVPKEY_Device_PDOName) && propType == DEVPROP_TYPE_STRING)
                physicalDeviceObjectName = ConvertMbcsToUtf8(ConvertBufferToString(buffer));
            if (IsEqualDevPropKey(key, DEVPKEY_Device_MatchingDeviceId) && propType == DEVPROP_TYPE_STRING)
                matchingDeviceId = ConvertMbcsToUtf8(ConvertBufferToString(buffer));
            if (IsEqualDevPropKey(key, DEVPKEY_Device_HardwareIds) && propType == DEVPROP_TYPE_STRING_LIST)
                hardwareIds = ConvertMbcsToUtf8(ConvertBufferToStringList(buffer));
        }

        if (deviceDescription && friendlyName && physicalDeviceObjectName && hardwareIds)
            descriptions.push_back({ *deviceDescription, *friendlyName, *physicalDeviceObjectName, *matchingDeviceId, *hardwareIds });
    }

    UartPortFinder::UartNotFound::UartNotFound(const std::string& portName)
        : std::runtime_error("Unable to find a UART with name " + portName)
    {}
}
