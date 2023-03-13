#ifndef SERVICES_GAP_HPP
#define SERVICES_GAP_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Observer.hpp"

namespace services
{
    enum class GapDeviceAddressType : uint8_t
    {
        publicAddress,
        randomAddress,
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

    enum class GapState : uint8_t
    {
        standby,
        scanning,
        advertising,
        connected
    };

    enum class GapAuthenticationErrorType : uint8_t
    {
        passkeyEntryFailed,
        authenticationRequirementsNotMet,
        pairingNotSupported,
        insufficientEncryptionKeySize,
        numericComparisonFailed,
        timeout,
        unknown,
    };

    enum class GapAdvertisingEventType : uint8_t
    {
        advInd,
        advDirectInd,
        advScanInd,
        advNonconnInd,
        scanResponse,
    };

    enum class GapAdvertisingEventAddressType : uint8_t
    {
        publicDeviceAddress,
        randomDeviceAddress,
        publicIdentityAddress,
        randomIdentityAddress
    };

    struct GapConnectionParameters
    {
        uint16_t minConnIntMultiplier;
        uint16_t maxConnIntMultiplier;
        uint16_t slaveLatency;
        uint16_t supervisorTimeoutMs;
    };

    struct GapAdvertisingReport
    {
        GapAdvertisingEventType eventType;
        GapAdvertisingEventAddressType addressType;
        hal::MacAddress address;
        infra::ConstByteRange data;
        int8_t rssi;
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

    class GapPeripheral;

    class GapPeripheralObserver
        : public infra::Observer<GapPeripheralObserver, GapPeripheral>
    {
    public:
        using infra::Observer<GapPeripheralObserver, GapPeripheral>::Observer;

        virtual void StateUpdated(GapState state) = 0;
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
        virtual void StateUpdated(GapState state) override;

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

    class GapCentral;

    class GapCentralObserver
        : public infra::Observer<GapCentralObserver, GapCentral>
    {
    public:
        using infra::Observer<GapCentralObserver, GapCentral>::Observer;

        virtual void AuthenticationSuccessfullyCompleted() = 0;
        virtual void AuthenticationFailed(GapAuthenticationErrorType error) = 0;
        virtual void DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered) = 0;
        virtual void StateUpdated(GapState state) = 0;
    };

    class GapCentral
        : public infra::Subject<GapCentralObserver>
    {
    public:
        virtual void Connect(hal::MacAddress macAddress, GapDeviceAddressType addressType) = 0;
        virtual void Disconnect() = 0;
        virtual void SetAddress(hal::MacAddress macAddress, GapDeviceAddressType addressType) = 0;
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
        virtual void AuthenticationSuccessfullyCompleted() override;
        virtual void AuthenticationFailed(GapAuthenticationErrorType error) override;
        virtual void DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered) override;
        virtual void StateUpdated(GapState state) override;

        // Implementation of GapCentral
        virtual void Connect(hal::MacAddress macAddress, GapDeviceAddressType addressType) override;
        virtual void Disconnect() override;
        virtual void SetAddress(hal::MacAddress macAddress, GapDeviceAddressType addressType) override;
        virtual void StartDeviceDiscovery() override;
        virtual void StopDeviceDiscovery() override;
    };
}

#endif
