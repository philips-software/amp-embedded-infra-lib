#ifndef SERVICES_GAP_HPP
#define SERVICES_GAP_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/Observer.hpp"

namespace services
{
    class Gap
    {
    public:
        enum class AdvDataType : uint8_t
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
            PublicTargetAddress = 0x17u,
            Appearance = 0x19u,
            AdvertisingInterval = 0x1au,
            LeRole = 0x1cu,
            ServSolicit_32BitUuidList = 0x1fu,
            Uri = 0x24u,
            ManufacturerSpecificData = 0xffu
        };

        using ScanRespDataType = AdvDataType;

        enum class AdvFlags : uint8_t
        {
            LeLimitedDiscMode = 0x01u,
            LeGeneralDiscMode = 0x02u,
            BrEdrNotSupported = 0x04u,
            LeBrEdrController = 0x08u,
            LeBrEdrHost = 0x10u
        };

        static constexpr uint8_t maxAdvertisementSize = 31;
        static constexpr uint8_t maxScanResponseSize = 31;
    };

    class GapPeripheral;

    enum class GapPeripheralState : uint8_t
    {
        Standby,
        Advertising,
        Connected
    };

    class GapPeripheralObserver
        : public infra::SingleObserver<GapPeripheralObserver, GapPeripheral>
    {
    public:
        using infra::SingleObserver<GapPeripheralObserver, GapPeripheral>::SingleObserver;

        virtual void State(GapPeripheralState state) = 0;
    };

    class GapPeripheral
        : public Gap
        , public infra::Subject<GapPeripheralObserver>
    {
    public:
        virtual hal::MacAddress GetPublicAddress() const = 0;
        virtual void SetAdvertisementData(infra::ConstByteRange data) = 0;
        virtual void SetScanResponseData(infra::ConstByteRange data) = 0;
        virtual void Advertise() = 0;
        virtual void Standby() = 0;
    };
}

#endif
