#ifndef HAL_UART_PORTFINDER_HPP
#define HAL_UART_PORTFINDER_HPP

#include <winsock2.h>
#include <setupapi.h>
#include <stdexcept>
#include <string>
#include <vector>


namespace hal
{
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
