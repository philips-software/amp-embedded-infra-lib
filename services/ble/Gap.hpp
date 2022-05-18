#ifndef SERVICES_GAP_HPP
#define SERVICES_GAP_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/Observer.hpp"

namespace services
{
    class Gap
    {
    public:
        using AdvIntMultiplier = uint16_t;

        enum class AdvType : uint8_t
        {
            advInd,
            advNonconnInd
        };

        enum class AdvDataType : uint8_t
        {
            flags = 0x01u,
            incmpltList16BitServUuid = 0x02u,
            cmpltList16BitServUuid = 0x03u,
            incmpltList32BitServUuid = 0x04u,
            cmpltList32BitServUuid = 0x05u,
            incmpltList128BitServUuid = 0x06u,
            cmpltList128BitServUuid = 0x07u,
            shortenedLocalName = 0x08u,
            completeLocalName = 0x09u,
            txPowerLevel = 0x0au,
            classOfDevice = 0x0du,
            secMgrTkValue = 0x10u,
            secMgrOobFlags = 0x11u,
            slaveConnInterval = 0x12u,
            servSolicit16BitUuidList = 0x14u,
            servSolicit128BitUuidList = 0x15u,
            serviceData = 0x16u,
            publicTargetAddress = 0x17u,
            appearance = 0x19u,
            advertisingInterval = 0x1au,
            leRole = 0x1cu,
            servSolicit32BitUuidList = 0x1fu,
            uri = 0x24u,
            manufacturerSpecificData = 0xffu
        };

        using ScanRespDataType = AdvDataType;

        enum class AdvFlags : uint8_t
        {
            leLimitedDiscMode = 0x01u,
            leGeneralDiscMode = 0x02u,
            brEdrNotSupported = 0x04u,
            leBrEdrController = 0x08u,
            leBrEdrHost = 0x10u
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
        virtual void Advertise(AdvType type, AdvIntMultiplier multiplier) = 0;
        virtual void Standby() = 0;
    };
}

#endif
