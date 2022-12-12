#ifndef SERVICES_UUID_HPP
#define SERVICES_UUID_HPP

#include "services/ble/Gatt.hpp"

namespace services
{
    namespace uuid
    {
        constexpr GattAttribute::Uuid16 deviceInformationService{ 0x180A };
        constexpr GattAttribute::Uuid16 systemId{ 0x2A23 };
        constexpr GattAttribute::Uuid16 modelNumber{ 0x2A24 };
        constexpr GattAttribute::Uuid16 serialNumber{ 0x2A25 };
        constexpr GattAttribute::Uuid16 firmwareRevision{ 0x2A26 };
        constexpr GattAttribute::Uuid16 hardwareRevision{ 0x2A27 };
        constexpr GattAttribute::Uuid16 softwareRevision{ 0x2A28 };
        constexpr GattAttribute::Uuid16 manufacturerName{ 0x2A29 };
        constexpr GattAttribute::Uuid16 ieeeCertification{ 0x2A2A };
        constexpr GattAttribute::Uuid16 pnpId{ 0x2A50 };

        constexpr GattAttribute::Uuid16 batteryService{ 0x180F };
        constexpr GattAttribute::Uuid16 batteryLevel{ 0x2A19 };
    }
}

#endif
