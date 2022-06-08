#ifndef SERVICES_GAP_HPP
#define SERVICES_GAP_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/Observer.hpp"

namespace services
{
    class Gap
    {
    public:
        using AdvertisementIntervalMultiplier = uint16_t; // Interval = Multiplier * 0.625 ms.
        static constexpr AdvertisementIntervalMultiplier advertisementIntervalMultiplierMin = 0x20u; // 20 ms
        static constexpr AdvertisementIntervalMultiplier advertisementIntervalMultiplierMax = 0x4000u; // 10240 ms

        enum class AdvertisementType : uint8_t
        {
            advInd,
            advNonconnInd
        };

        enum class AdvertisementDataType : uint8_t
        {
            flags = 0x01u,
            completeLocalName = 0x09u,
            publicTargetAddress = 0x17u,
            manufacturerSpecificData = 0xffu
        };

        using ScanResponseDataType = AdvertisementDataType;

        enum class AdvertisementFlags : uint8_t
        {
            leLimitedDiscoverableMode = 0x01u,
            leGeneralDiscoverableMode = 0x02u,
            brEdrNotSupported = 0x04u,
            leBrEdrController = 0x08u,
            leBrEdrHost = 0x10u
        };

        static constexpr uint8_t maxAdvertisementDataSize = 31;
        static constexpr uint8_t maxScanResponseDataSize = 31;
    };

    class GapPeripheralPairing
    {
    public:
        virtual void AllowPairing(bool allow) = 0;
    };

    class GapPeripheralBonding
    {
    public:
        virtual void RemoveAllBonds() = 0;
    };

    enum class GapPeripheralState : uint8_t
    {
        Standby,
        Advertising,
        Connected
    };

    class GapPeripheral;

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
        virtual void Advertise(AdvertisementType type, AdvertisementIntervalMultiplier multiplier) = 0;
        virtual void Standby() = 0;
    };
}

#endif
