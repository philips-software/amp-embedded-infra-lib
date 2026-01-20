#ifndef SERVICES_GAP_HPP
#define SERVICES_GAP_HPP

#include "hal/interfaces/MacAddress.hpp"
#include "infra/timer/Timer.hpp"
#include "infra/util/BoundedVector.hpp"
#include "infra/util/ByteRange.hpp"
#include "infra/util/EnumCast.hpp"
#include "infra/util/Observer.hpp"
#include "services/ble/Att.hpp"
#include <optional>

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

    enum class GapState : uint8_t
    {
        standby,
        scanning,
        advertising,
        connected,
        initiating
    };

    enum class GapAdvertisingEventType : uint8_t
    {
        advInd,
        advDirectInd,
        advScanInd,
        advNonconnInd,
        scanResponse,
    };

    enum class GapAdvertisementDataType : uint8_t
    {
        unknownType = 0x00u,
        flags = 0x01u,
        completeListOf16BitUuids = 0x03u,
        completeListOf128BitUuids = 0x07u,
        shortenedLocalName = 0x08u,
        completeLocalName = 0x09u,
        publicTargetAddress = 0x17u,
        appearance = 0x19u,
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

    struct GapAddress
    {
        hal::MacAddress address;
        GapDeviceAddressType type;

        bool operator==(GapAddress const& rhs) const
        {
            return type == rhs.type && address == rhs.address;
        }
    };

    struct GapOutOfBandData
    {
        hal::MacAddress macAddress;
        GapDeviceAddressType addressType;
        infra::ConstByteRange randomData;
        infra::ConstByteRange confirmData;
    };

    class GapPairing;

    class GapPairingObserver
        : public infra::Observer<GapPairingObserver, GapPairing>
    {
    public:
        using infra::Observer<GapPairingObserver, GapPairing>::Observer;

        enum class PairingErrorType : uint8_t
        {
            passkeyEntryFailed,
            authenticationRequirementsNotMet,
            pairingNotSupported,
            insufficientEncryptionKeySize,
            numericComparisonFailed,
            timeout,
            encryptionFailed,
            unknown,
        };

        virtual void DisplayPasskey(int32_t passkey, bool numericComparison) = 0;
        virtual void PairingSuccessfullyCompleted() = 0;
        virtual void PairingFailed(PairingErrorType error) = 0;
        virtual void OutOfBandDataGenerated(const GapOutOfBandData& outOfBandData) = 0;
    };

    class GapPairing
        : public infra::Subject<GapPairingObserver>
    {
    public:
        enum class IoCapabilities : uint8_t
        {
            display,
            displayYesNo,
            keyboard,
            none,
            keyboardDisplay
        };

        enum class SecurityMode : uint8_t
        {
            mode1,
            mode2
        };

        enum class SecurityLevel : uint8_t
        {
            level1,
            level2,
            level3,
            level4,
        };

        // 1. If there is a pre-existing bond, then the connection will be encrypted.
        // 2. If there is no pre-existing bond, then pairing, encrypting, and bonding (storing the keys) will take place.
        virtual void PairAndBond() = 0;

        virtual void AllowPairing(bool allow) = 0;
        virtual void SetSecurityMode(SecurityMode mode, SecurityLevel level) = 0;
        virtual void SetIoCapabilities(IoCapabilities caps) = 0;
        virtual void GenerateOutOfBandData() = 0;
        virtual void SetOutOfBandData(const GapOutOfBandData& outOfBandData) = 0;
        virtual void AuthenticateWithPasskey(uint32_t passkey) = 0;
        virtual void NumericComparisonConfirm(bool accept) = 0;
    };

    class GapPairingDecorator
        : public GapPairingObserver
        , public GapPairing
    {
    public:
        using GapPairingObserver::GapPairingObserver;

        // Implementation of GapPairingObserver
        void DisplayPasskey(int32_t passkey, bool numericComparison) override;
        void PairingSuccessfullyCompleted() override;
        void PairingFailed(PairingErrorType error) override;
        void OutOfBandDataGenerated(const GapOutOfBandData& outOfBandData) override;

        // Implementation of GapPairing
        void PairAndBond() override;
        void AllowPairing(bool allow) override;
        void SetSecurityMode(SecurityMode mode, SecurityLevel level) override;
        void SetIoCapabilities(IoCapabilities caps) override;
        void GenerateOutOfBandData() override;
        void SetOutOfBandData(const GapOutOfBandData& outOfBandData) override;
        void AuthenticateWithPasskey(uint32_t passkey) override;
        void NumericComparisonConfirm(bool accept) override;
    };

    class GapBonding;

    class GapBondingObserver
        : public infra::Observer<GapBondingObserver, GapBonding>
    {
    public:
        using infra::Observer<GapBondingObserver, GapBonding>::Observer;

        virtual void NumberOfBondsChanged(std::size_t nrBonds) = 0;
    };

    class GapBonding
        : public infra::Subject<GapBondingObserver>
    {
    public:
        virtual void RemoveAllBonds() = 0;
        virtual void RemoveOldestBond() = 0;

        virtual std::size_t GetMaxNumberOfBonds() const = 0;
        virtual std::size_t GetNumberOfBonds() const = 0;
        virtual bool IsDeviceBonded(hal::MacAddress address, GapDeviceAddressType addressType) const = 0;
    };

    class GapBondingDecorator
        : public GapBondingObserver
        , public GapBonding
    {
    public:
        using GapBondingObserver::GapBondingObserver;

        // Implementation of GapBondingObserver
        void NumberOfBondsChanged(std::size_t nrBonds) override;

        // Implementation of GapBonding
        void RemoveAllBonds() override;
        void RemoveOldestBond() override;
        std::size_t GetMaxNumberOfBonds() const override;
        std::size_t GetNumberOfBonds() const override;
        bool IsDeviceBonded(hal::MacAddress address, GapDeviceAddressType addressType) const override;
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
        virtual void SetConnectionParameters(const services::GapConnectionParameters& connParam) = 0;
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
        void SetConnectionParameters(const services::GapConnectionParameters& connParam) override;
    };

    inline GapPeripheral::AdvertisementFlags operator|(GapPeripheral::AdvertisementFlags lhs, GapPeripheral::AdvertisementFlags rhs)
    {
        return static_cast<GapPeripheral::AdvertisementFlags>(infra::enum_cast(lhs) | infra::enum_cast(rhs));
    }

    class GapAdvertisingDataParser
    {
    public:
        explicit GapAdvertisingDataParser(infra::ConstByteRange data);

        infra::ConstByteRange LocalName() const;
        std::optional<std::pair<uint16_t, infra::ConstByteRange>> ManufacturerSpecificData() const;
        std::optional<GapPeripheral::AdvertisementFlags> Flags() const;
        infra::MemoryRange<const AttAttribute::Uuid16> CompleteListOf16BitUuids() const;
        infra::MemoryRange<const AttAttribute::Uuid128> CompleteListOf128BitUuids() const;
        std::optional<uint16_t> Appearance() const;

    private:
        infra::ConstByteRange data;

    private:
        infra::ConstByteRange ParserAdvertisingData(GapAdvertisementDataType type) const;
    };

    class GapAdvertisementFormatter
    {
    public:
        explicit GapAdvertisementFormatter(infra::BoundedVector<uint8_t>& payload);

        void AppendFlags(GapPeripheral::AdvertisementFlags flags);
        void AppendCompleteLocalName(const infra::BoundedConstString& name);
        void AppendShortenedLocalName(const infra::BoundedConstString& name);
        void AppendManufacturerData(uint16_t manufacturerCode, infra::ConstByteRange data);
        void AppendListOfServicesUuid(infra::MemoryRange<AttAttribute::Uuid16> services);
        void AppendListOfServicesUuid(infra::MemoryRange<AttAttribute::Uuid128> services);
        void AppendPublicTargetAddress(hal::MacAddress address);
        void AppendAppearance(uint16_t appearance);

        infra::ConstByteRange FormattedAdvertisementData() const;
        std::size_t RemainingSpaceAvailable() const;

    private:
        static constexpr std::size_t headerSize = 2;

        infra::BoundedVector<uint8_t>& payload;
    };

    struct GapAdvertisingReport
    {
        GapAdvertisingEventType eventType;
        GapDeviceAddressType addressType;
        hal::MacAddress address;
        infra::BoundedVector<uint8_t>::WithMaxSize<GapPeripheral::maxAdvertisementDataSize> data;
        int32_t rssi;
    };

    class GapCentral;

    class GapCentralObserver
        : public infra::Observer<GapCentralObserver, GapCentral>
    {
    public:
        using infra::Observer<GapCentralObserver, GapCentral>::Observer;

        virtual void DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered) = 0;
        virtual void StateChanged(GapState state) = 0;
    };

    class GapCentral
        : public infra::Subject<GapCentralObserver>
    {
    public:
        virtual void Connect(hal::MacAddress macAddress, GapDeviceAddressType addressType, infra::Duration initiatingTimeout) = 0;
        virtual void CancelConnect() = 0;
        virtual void Disconnect() = 0;
        virtual void SetAddress(hal::MacAddress macAddress, GapDeviceAddressType addressType) = 0;
        virtual void StartDeviceDiscovery() = 0;
        virtual void StopDeviceDiscovery() = 0;
        virtual std::optional<hal::MacAddress> ResolvePrivateAddress(hal::MacAddress address) const = 0;
    };

    class GapCentralDecorator
        : public GapCentralObserver
        , public GapCentral
    {
    public:
        using GapCentralObserver::GapCentralObserver;

        // Implementation of GapCentralObserver
        void DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered) override;
        void StateChanged(GapState state) override;

        // Implementation of GapCentral
        void Connect(hal::MacAddress macAddress, GapDeviceAddressType addressType, infra::Duration initiatingTimeout) override;
        void CancelConnect() override;
        void Disconnect() override;
        void SetAddress(hal::MacAddress macAddress, GapDeviceAddressType addressType) override;
        void StartDeviceDiscovery() override;
        void StopDeviceDiscovery() override;
        std::optional<hal::MacAddress> ResolvePrivateAddress(hal::MacAddress address) const override;
    };
}

namespace infra
{
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::GapAdvertisingEventType& eventType);
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::GapDeviceAddressType& addressType);
    infra::TextOutputStream& operator<<(infra::TextOutputStream& stream, const services::GapState& state);
}

#endif
