#ifndef SERVICES_GAP_HPP
#define SERVICES_GAP_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Observer.hpp"

namespace services
{
    enum class GapAddressType : uint8_t
    {
        Public,
        Random
    };

    enum class GapAdvertisementType : uint8_t
    {
        AdvInd,
        AdvNonconnInd
    };

    enum class GapIoCapabilities
    {
        Display,
        DisplayYesNo,
        Keyboard,
        None,
        KeyboardDisplay
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
        : public infra::Observer<GapPeripheralObserver, GapPeripheral>
    {
    public:
        using infra::Observer<GapPeripheralObserver, GapPeripheral>::Observer;

        virtual void StateUpdated(GapPeripheralState state) = 0;
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
        virtual hal::MacAddress GetResolvableAddress() const = 0;
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
        virtual void StateUpdated(GapPeripheralState state) override;

        // Implementation of GapPeripheral
        virtual hal::MacAddress GetResolvableAddress() const override;
        virtual void SetAdvertisementData(infra::ConstByteRange data) override;
        virtual void SetScanResponseData(infra::ConstByteRange data) override;
        virtual void Advertise(GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier) override;
        virtual void Standby() override;
    };

    inline GapPeripheral::AdvertisementFlags operator|(GapPeripheral::AdvertisementFlags lhs, GapPeripheral::AdvertisementFlags rhs)
    {
        return static_cast<GapPeripheral::AdvertisementFlags>(infra::enum_cast(lhs) | infra::enum_cast(rhs));
    }

    enum class GapCentralState
    {
        Standby,
        Connected,
        Disconnected,
        Scanning,
    };

    enum class GapCentralAuthenticationStatus
    {
        Success,
        PasskeyEntryFailed,
        AuthenticationRequirementsNotMet,
        PairingNotSupported,
        InsufficientEncryptionKeySize,
        NumericComparisonFailed,
        Timeout,
        Unknown,
    };

    enum class GapCentralSecurityLevel
    {
        None,
        UnauthenticatedPairing,
        AuthenticatedPairing,
        AuthenticatedPairingWithLE
    };

    enum class GapCentralPairingMode : uint8_t
    {
        Pair,
        Bond,
        None
    };

    struct GapCentralConnectionParameters
    {
        uint16_t minConnIntMultiplier;
        uint16_t maxConnIntMultiplier;
        uint16_t slaveLatency;
        uint16_t supervisorTimeoutMs;
    };

    struct GapCentralDiscoveryParameters
    {
        hal::MacAddress address;
        GapAddressType addressType;
        infra::ConstByteRange data;
        int8_t rssi;
        bool isScanResponse;
        GapAdvertisementType advertisementType;
    };

    class GapCentral;

    class GapCentralObserver
        : public infra::Observer<GapCentralObserver, GapCentral>
    {
    public:
        using infra::Observer<GapCentralObserver, GapCentral>::Observer;

        virtual void AuthenticationComplete(GapCentralAuthenticationStatus status) = 0;
        virtual void DeviceDiscovered(const GapCentralDiscoveryParameters& deviceDiscovered) = 0;
        virtual void StateUpdated(GapCentralState state) = 0;
    };

    class GapCentral
        : public infra::Subject<GapCentralObserver>
    {
    public:
        virtual void Connect(hal::MacAddress macAddress, GapAddressType addressType) = 0;
        virtual void Disconnect() = 0;
        virtual void SetAddress(hal::MacAddress macAddress, GapAddressType addressType) = 0;
        virtual void StartDeviceDiscovery() = 0;
        virtual void StopDeviceDiscovery() = 0;
    };

    class GapCentralDecorator
        : public GapCentralObserver
        , public GapCentral
    {
    public:
        using GapCentralObserver::GapCentralObserver;

        // Implementation of GapCentralObserver
        virtual void AuthenticationComplete(GapCentralAuthenticationStatus status) override;
        virtual void DeviceDiscovered(const GapCentralDiscoveryParameters& deviceDiscovered) override;
        virtual void StateUpdated(GapCentralState state) override;

        // Implementation of GapCentral
        virtual void Connect(hal::MacAddress macAddress, GapAddressType addressType) override;
        virtual void Disconnect() override;
        virtual void SetAddress(hal::MacAddress macAddress, GapAddressType addressType) override;
        virtual void StartDeviceDiscovery() override;
        virtual void StopDeviceDiscovery() override;
    };
}

#endif
