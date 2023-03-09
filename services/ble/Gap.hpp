#ifndef SERVICES_GAP_HPP
#define SERVICES_GAP_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Observer.hpp"

namespace services
{
    enum class GapAddressType : uint8_t
    {
        publicAddress,
        randomAddress
    };
    
    enum class GapAdvertisementType : uint8_t
    {
        advInd,
        advNonconnInd
    };

    enum class GapIoCapabilities : uint8_t
    {
        display,
        displayYesNo,
        keyboard,
        none,
        keyboardDisplay
    };

    enum class GapSecurityMode : uint8_t
    {
        mode1,
        mode2
    };

    enum class GapSecurityLevel : uint8_t
    {
        level1,
        level2,
        level3,
        level4,
    };

    struct GapAddress
    {

        hal::MacAddress address;
        GapAddressType type;
    };

    class GapPeripheralPairing
    {
    public:
        virtual void AllowPairing(bool allow) = 0;

        virtual void AuthenticateWithPasskey(uint32_t passkey) = 0;
        virtual bool NumericComparisonConfirm(bool accept) = 0;
    };

    class GapPeripheralBonding;

    class GapPeripheralBondingObserver
        : public infra::Observer<GapPeripheralBondingObserver, GapPeripheralBonding>
    {
    public:
        using infra::Observer<GapPeripheralBondingObserver, GapPeripheralBonding>::Observer;

        virtual void NrOfBondsChanged(size_t nrBonds) = 0;
    };

    class GapPeripheralBonding
        : public infra::Subject<GapPeripheralBondingObserver>
    {
    public:
        virtual void RemoveAllBonds() = 0;
        virtual void RemoveOldestBond() = 0;

        virtual size_t GetMaxNrBonds() const = 0;
        virtual size_t GetNrBonds() const = 0;
    };


    enum class GapPeripheralState : uint8_t
    {
        standby,
        advertising,
        connected
    };

    class GapPeripheral;

    class GapPeripheralObserver
        : public infra::Observer<GapPeripheralObserver, GapPeripheral>
    {
    public:
        using infra::Observer<GapPeripheralObserver, GapPeripheral>::Observer;

        virtual void StateChanged(GapPeripheralState state) = 0;
    };

    class GapPeripheral
        : public infra::Subject<GapPeripheralObserver>
    {
    public:
        using AdvertisementIntervalMultiplier = uint16_t;                                              // Interval = Multiplier * 0.625 ms.
        static constexpr AdvertisementIntervalMultiplier advertisementIntervalMultiplierMin = 0x20u;   // 20 ms
        static constexpr AdvertisementIntervalMultiplier advertisementIntervalMultiplierMax = 0x4000u; // 10240 ms

        enum class AdvertisementDataType : uint8_t
        {
            flags = 0x01u,
            completeListOf128BitUuids = 0x07u,
            completeLocalName = 0x09u,
            publicTargetAddress = 0x17u,
            manufacturerSpecificData = 0xffu
        };

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
        static constexpr uint16_t connectionInitialMaxTxOctets = 251;
        static constexpr uint16_t connectionInitialMaxTxTime = 2120; // (connectionInitialMaxTxOctets + 14) * 8

    public:

        virtual GapAddress GetAddress() const = 0;
        virtual void SetAdvertisementData(infra::ConstByteRange data) = 0;
        virtual void SetScanResponseData(infra::ConstByteRange data) = 0;
        virtual void Advertise(GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier) = 0;
        virtual void Standby() = 0;
    };

    class GapPeripheralDecorator
        : public GapPeripheralObserver
        , public GapPeripheral
    {
    public:
        using GapPeripheralObserver::GapPeripheralObserver;

        // Implementation of GapPeripheralObserver
        virtual void StateChanged(GapPeripheralState state) override;

        // Implementation of GapPeripheral
        virtual GapAddress GetAddress() const override;
        virtual void SetAdvertisementData(infra::ConstByteRange data) override;
        virtual void SetScanResponseData(infra::ConstByteRange data) override;
        virtual void Advertise(GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier) override;
        virtual void Standby() override;
    };

    inline GapPeripheral::AdvertisementFlags operator|(GapPeripheral::AdvertisementFlags lhs, GapPeripheral::AdvertisementFlags rhs)
    {
        return static_cast<GapPeripheral::AdvertisementFlags>(infra::enum_cast(lhs) | infra::enum_cast(rhs));
    }
}

#endif
