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

    enum class GapAdvertisementDataType : uint8_t
    {
        unknownType = 0x00u,
        flags = 0x01u,
        completeListOf128BitUuids = 0x07u,
        shortenedLocalName = 0x08u,
        completeLocalName = 0x09u,
        publicTargetAddress = 0x17u,
        manufacturerSpecificData = 0xffu
    };

    struct GapConnectionParameters
    {
        uint16_t minConnIntMultiplier;
        uint16_t maxConnIntMultiplier;
        uint16_t slaveLatency;
        uint16_t supervisorTimeoutMs;

        static constexpr uint16_t connectionInitialMaxTxOctets = 251;
        static constexpr uint16_t connectionInitialMaxTxTime = 2120; // (connectionInitialMaxTxOctets + 14) * 8
    };

    struct GapAdvertisingReport
    {
        GapAdvertisingEventType eventType;
        GapAdvertisingEventAddressType addressType;
        hal::MacAddress address;
        infra::ConstByteRange data;
        int8_t rssi;
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
        GapDeviceAddressType type;

        bool operator==(GapAddress const& rhs) const
        {
            return type == rhs.type && address == rhs.address;
        }
    };

    class GapAdvertisingDataParser
    {
    public:
        explicit GapAdvertisingDataParser(infra::ConstByteRange data);

        infra::ConstByteRange LocalName();
        infra::ConstByteRange ManufacturerSpecificData();

    private:
        infra::ConstByteRange data;

    private:
        infra::ConstByteRange ParserAdvertisingData(GapAdvertisementDataType type) const;
    };

    class GapPeripheralPairing;

    class GapPeripheralPairingObserver
        : public infra::Observer<GapPeripheralPairingObserver, GapPeripheralPairing>
    {
    public:
        using infra::Observer<GapPeripheralPairingObserver, GapPeripheralPairing>::Observer;

        virtual void DisplayPasskey(int32_t passkey, bool numericComparison) = 0;
    };

    class GapPeripheralPairing
        : public infra::Subject<GapPeripheralPairingObserver>
    {
    public:
        virtual void AllowPairing(bool allow) = 0;

        virtual void SetSecurityMode(GapSecurityMode mode, GapSecurityLevel level) = 0;
        virtual void SetIoCapabilities(GapIoCapabilities caps) = 0;

        virtual void AuthenticateWithPasskey(uint32_t passkey) = 0;
        virtual void NumericComparisonConfirm(bool accept) = 0;
    };

    class GapPeripheralBonding;

    class GapPeripheralBondingObserver
        : public infra::Observer<GapPeripheralBondingObserver, GapPeripheralBonding>
    {
    public:
        using infra::Observer<GapPeripheralBondingObserver, GapPeripheralBonding>::Observer;

        virtual void NumberOfBondsChanged(size_t nrBonds) = 0;
    };

    class GapPeripheralBonding
        : public infra::Subject<GapPeripheralBondingObserver>
    {
    public:
        virtual void RemoveAllBonds() = 0;
        virtual void RemoveOldestBond() = 0;

        virtual size_t GetMaxNumberOfBonds() const = 0;
        virtual size_t GetNumberOfBonds() const = 0;
    };

    class GapPeripheral;

    class GapPeripheralObserver
        : public infra::Observer<GapPeripheralObserver, GapPeripheral>
    {
    public:
        using infra::Observer<GapPeripheralObserver, GapPeripheral>::Observer;

        virtual void StateChanged(GapState state) = 0;
    };

    class GapPeripheral
        : public infra::Subject<GapPeripheralObserver>
    {
    public:
        using AdvertisementIntervalMultiplier = uint16_t;                                              // Interval = Multiplier * 0.625 ms.
        static constexpr AdvertisementIntervalMultiplier advertisementIntervalMultiplierMin = 0x20u;   // 20 ms
        static constexpr AdvertisementIntervalMultiplier advertisementIntervalMultiplierMax = 0x4000u; // 10240 ms

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

    public:
        virtual GapAddress GetAddress() const = 0;
        virtual GapAddress GetIdentityAddress() const = 0;
        virtual void SetAdvertisementData(infra::ConstByteRange data) = 0;
        virtual infra::ConstByteRange GetAdvertisementData() const = 0;
        virtual void SetScanResponseData(infra::ConstByteRange data) = 0;
        virtual infra::ConstByteRange GetScanResponseData() const = 0;
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
        void StateChanged(GapState state) override;

        // Implementation of GapPeripheral
        GapAddress GetAddress() const override;
        GapAddress GetIdentityAddress() const override;

        void SetAdvertisementData(infra::ConstByteRange data) override;
        infra::ConstByteRange GetAdvertisementData() const override;
        void SetScanResponseData(infra::ConstByteRange data) override;
        infra::ConstByteRange GetScanResponseData() const override;
        void Advertise(GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier) override;
        void Standby() override;
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
        virtual void StateChanged(GapState state) = 0;
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
        void AuthenticationSuccessfullyCompleted() override;
        void AuthenticationFailed(GapAuthenticationErrorType error) override;
        void DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered) override;
        void StateChanged(GapState state) override;

        // Implementation of GapCentral
        void Connect(hal::MacAddress macAddress, GapDeviceAddressType addressType) override;
        void Disconnect() override;
        void SetAddress(hal::MacAddress macAddress, GapDeviceAddressType addressType) override;
        void StartDeviceDiscovery() override;
        void StopDeviceDiscovery() override;
    };
}

namespace infra
{
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::GapAdvertisingEventType& eventType);
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::GapAdvertisingEventAddressType& addressType);
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::GapState& state);
}

#endif
