#ifndef BLE_CORE_SPECIFICATION_DEFINITIONS_HPP
#define BLE_CORE_SPECIFICATION_DEFINITIONS_HPP

#include <cstdint>

namespace services
{
    class Ble
    {
    public:
        //Core Specification Supplement, Part A, Section 1.3
        enum class ADType : uint8_t
        {
            Flags = 0x01u,
            IncmpltList16BitServUuid = 0x02u,
            CmpltList16BitServUuid = 0x03u,
            IncmpltList32BitServUuid = 0x04u,
            CmpltList32BitServUuid = 0x05u,
            IncmpltList128BitServUuid = 0x06u,
            CmpltList128BitServUuid = 0x07u,
            ShortenedLocalName = 0x08u,
            CompleteLocalName = 0x09u,
            TxPowerLevel = 0x0au,
            ClassOfDevice = 0x0du,
            SecMgrTkValue = 0x10u,
            SecMgrOobFlags = 0x11u,
            SlaveConnInterval = 0x12u,
            ServSolicit16BitUuidList = 0x14u,
            ServSolicit128BitUuidList = 0x15u,
            ServiceData = 0x16u,
            Appearance = 0x19u,
            AdvertisingInterval = 0x1au,
            LeRole = 0x1cu,
            ServSolicit_32BitUuidList = 0x1fu,
            Uri = 0x24u,
            ManufacturerSpecificData = 0xffu
        };

        //Core Specification Supplement, Part A, Section 1.3
        typedef ADType SRDType;

        static constexpr uint8_t MaxAdvertisementSize = 31;
        static constexpr uint8_t MaxScanResponseSize = 31;
    };
}

#endif
