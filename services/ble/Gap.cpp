#include "services/ble/Gap.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/MemoryRange.hpp"
#include <optional>

namespace
{
    void AddHeader(infra::BoundedVector<uint8_t>& payload, std::size_t length, services::GapAdvertisementDataType type)
    {
        payload.push_back(static_cast<uint8_t>(length + 1));
        payload.push_back(static_cast<uint8_t>(type));
    }

    void AddData(infra::BoundedVector<uint8_t>& payload, infra::ConstByteRange data)
    {
        payload.insert(payload.end(), data.begin(), data.end());
    }
}

namespace services
{
    void GapPairingDecorator::DisplayPasskey(int32_t passkey, bool numericComparison)
    {
        GapPairing::NotifyObservers([&passkey, &numericComparison](auto& obs)
            {
                obs.DisplayPasskey(passkey, numericComparison);
            });
    }

    void GapPairingDecorator::PairingSuccessfullyCompleted()
    {
        GapPairing::NotifyObservers([](auto& obs)
            {
                obs.PairingSuccessfullyCompleted();
            });
    }

    void GapPairingDecorator::PairingFailed(PairingErrorType error)
    {
        GapPairing::NotifyObservers([&error](auto& obs)
            {
                obs.PairingFailed(error);
            });
    }

    void GapPairingDecorator::OutOfBandDataGenerated(const GapOutOfBandData& outOfBandData)
    {
        GapPairing::NotifyObservers([outOfBandData](auto& obs)
            {
                obs.OutOfBandDataGenerated(outOfBandData);
            });
    }

    void GapPairingDecorator::PairAndBond()
    {
        GapPairingObserver::Subject().PairAndBond();
    }

    void GapPairingDecorator::AllowPairing(bool allow)
    {
        GapPairingObserver::Subject().AllowPairing(allow);
    }

    void GapPairingDecorator::SetSecurityMode(SecurityMode mode, SecurityLevel level)
    {
        GapPairingObserver::Subject().SetSecurityMode(mode, level);
    }

    void GapPairingDecorator::SetIoCapabilities(IoCapabilities caps)
    {
        GapPairingObserver::Subject().SetIoCapabilities(caps);
    }

    void GapPairingDecorator::GenerateOutOfBandData()
    {
        GapPairingObserver::Subject().GenerateOutOfBandData();
    }

    void GapPairingDecorator::SetOutOfBandData(const GapOutOfBandData& outOfBandData)
    {
        GapPairingObserver::Subject().SetOutOfBandData(outOfBandData);
    }

    void GapPairingDecorator::AuthenticateWithPasskey(uint32_t passkey)
    {
        GapPairingObserver::Subject().AuthenticateWithPasskey(passkey);
    }

    void GapPairingDecorator::NumericComparisonConfirm(bool accept)
    {
        GapPairingObserver::Subject().NumericComparisonConfirm(accept);
    }

    void GapBondingDecorator::NumberOfBondsChanged(std::size_t nrBonds)
    {
        GapBonding::NotifyObservers([&nrBonds](auto& obs)
            {
                obs.NumberOfBondsChanged(nrBonds);
            });
    }

    void GapBondingDecorator::RemoveAllBonds()
    {
        GapBondingObserver::Subject().RemoveAllBonds();
    }

    void GapBondingDecorator::RemoveOldestBond()
    {
        GapBondingObserver::Subject().RemoveOldestBond();
    }

    std::size_t GapBondingDecorator::GetMaxNumberOfBonds() const
    {
        return GapBondingObserver::Subject().GetMaxNumberOfBonds();
    }

    std::size_t GapBondingDecorator::GetNumberOfBonds() const
    {
        return GapBondingObserver::Subject().GetNumberOfBonds();
    }

    bool GapBondingDecorator::IsDeviceBonded(hal::MacAddress address, GapDeviceAddressType addressType) const
    {
        return GapBondingObserver::Subject().IsDeviceBonded(address, addressType);
    }

    void GapPeripheralDecorator::StateChanged(GapState state)
    {
        GapPeripheral::NotifyObservers([&state](auto& obs)
            {
                obs.StateChanged(state);
            });
    }

    GapAddress GapPeripheralDecorator::GetAddress() const
    {
        return GapPeripheralObserver::Subject().GetAddress();
    }

    GapAddress GapPeripheralDecorator::GetIdentityAddress() const
    {
        return GapPeripheralObserver::Subject().GetIdentityAddress();
    }

    void GapPeripheralDecorator::SetAdvertisementData(infra::ConstByteRange data)
    {
        GapPeripheralObserver::Subject().SetAdvertisementData(data);
    }

    infra::ConstByteRange GapPeripheralDecorator::GetAdvertisementData() const
    {
        return GapPeripheralObserver::Subject().GetAdvertisementData();
    }

    void GapPeripheralDecorator::SetScanResponseData(infra::ConstByteRange data)
    {
        GapPeripheralObserver::Subject().SetScanResponseData(data);
    }

    infra::ConstByteRange GapPeripheralDecorator::GetScanResponseData() const
    {
        return GapPeripheralObserver::Subject().GetScanResponseData();
    }

    void GapPeripheralDecorator::Advertise(GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier)
    {
        GapPeripheralObserver::Subject().Advertise(type, multiplier);
    }

    void GapPeripheralDecorator::Standby()
    {
        GapPeripheralObserver::Subject().Standby();
    }

    void GapPeripheralDecorator::SetConnectionParameters(const services::GapConnectionParameters& connParam)
    {
        GapPeripheralObserver::Subject().SetConnectionParameters(connParam);
    }

    void GapCentralDecorator::DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered)
    {
        GapCentralObserver::SubjectType::NotifyObservers([&deviceDiscovered](auto& obs)
            {
                obs.DeviceDiscovered(deviceDiscovered);
            });
    }

    void GapCentralDecorator::StateChanged(GapState state)
    {
        GapCentralObserver::SubjectType::NotifyObservers([&state](auto& obs)
            {
                obs.StateChanged(state);
            });
    }

    void GapCentralDecorator::Connect(hal::MacAddress macAddress, GapDeviceAddressType addressType, infra::Duration initiatingTimeout)
    {
        GapCentralObserver::Subject().Connect(macAddress, addressType, initiatingTimeout);
    }

    void GapCentralDecorator::CancelConnect()
    {
        GapCentralObserver::Subject().CancelConnect();
    }

    void GapCentralDecorator::Disconnect()
    {
        GapCentralObserver::Subject().Disconnect();
    }

    void GapCentralDecorator::SetAddress(hal::MacAddress macAddress, GapDeviceAddressType addressType)
    {
        GapCentralObserver::Subject().SetAddress(macAddress, addressType);
    }

    void GapCentralDecorator::StartDeviceDiscovery()
    {
        GapCentralObserver::Subject().StartDeviceDiscovery();
    }

    void GapCentralDecorator::StopDeviceDiscovery()
    {
        GapCentralObserver::Subject().StopDeviceDiscovery();
    }

    std::optional<hal::MacAddress> GapCentralDecorator::ResolvePrivateAddress(hal::MacAddress address) const
    {
        return GapCentralObserver::Subject().ResolvePrivateAddress(address);
    }

    GapAdvertisingDataParser::GapAdvertisingDataParser(infra::ConstByteRange data)
        : data(data)
    {}

    infra::ConstByteRange GapAdvertisingDataParser::LocalName() const
    {
        auto localName = ParserAdvertisingData(GapAdvertisementDataType::completeLocalName);

        if (localName.empty())
            return ParserAdvertisingData(GapAdvertisementDataType::shortenedLocalName);
        else
            return localName;
    }

    std::optional<std::pair<uint16_t, infra::ConstByteRange>> GapAdvertisingDataParser::ManufacturerSpecificData() const
    {
        infra::ByteInputStream stream(ParserAdvertisingData(GapAdvertisementDataType::manufacturerSpecificData), infra::softFail);
        auto manufacturerCode = stream.Extract<uint16_t>();
        auto manufacturerData = stream.Reader().Remaining();

        if (stream.Failed())
            return std::nullopt;

        return std::make_optional(std::make_pair(manufacturerCode, manufacturerData));
    }

    std::optional<GapPeripheral::AdvertisementFlags> GapAdvertisingDataParser::Flags() const
    {
        auto flagsData = ParserAdvertisingData(GapAdvertisementDataType::flags);

        if (flagsData.empty())
            return std::nullopt;

        if (flagsData.size() != 1)
            return std::nullopt;

        return std::make_optional(static_cast<GapPeripheral::AdvertisementFlags>(flagsData[0]));
    }

    infra::MemoryRange<const AttAttribute::Uuid16> GapAdvertisingDataParser::CompleteListOf16BitUuids() const
    {
        auto uuidData = ParserAdvertisingData(GapAdvertisementDataType::completeListOf16BitUuids);

        if (uuidData.size() % sizeof(AttAttribute::Uuid16) != 0)
            return {};

        return infra::ConstCastMemoryRange<AttAttribute::Uuid16>(infra::ReinterpretCastMemoryRange<const AttAttribute::Uuid16>(uuidData));
    }

    infra::MemoryRange<const AttAttribute::Uuid128> GapAdvertisingDataParser::CompleteListOf128BitUuids() const
    {
        auto uuidData = ParserAdvertisingData(GapAdvertisementDataType::completeListOf128BitUuids);

        if (uuidData.size() % sizeof(AttAttribute::Uuid128) != 0)
            return {};

        return infra::ConstCastMemoryRange<AttAttribute::Uuid128>(infra::ReinterpretCastMemoryRange<const AttAttribute::Uuid128>(uuidData));
    }

    std::optional<uint16_t> GapAdvertisingDataParser::Appearance() const
    {
        auto appearanceData = ParserAdvertisingData(GapAdvertisementDataType::appearance);

        infra::ByteInputStream stream(appearanceData, infra::softFail);
        auto appearance = stream.Extract<uint16_t>();

        if (stream.Failed())
            return std::nullopt;

        return std::make_optional(appearance);
    }

    infra::ConstByteRange GapAdvertisingDataParser::ParserAdvertisingData(GapAdvertisementDataType type) const
    {
        const uint8_t lengthOffset = 0;
        const uint8_t advertisingTypeOffset = 1;
        const uint8_t headerSize = 2;

        infra::ConstByteRange advData = data;

        while (!advData.empty())
        {
            size_t elementSize = std::min<size_t>(advData[lengthOffset] + 1, advData.size());

            auto element = infra::Head(advData, elementSize);

            if (element.size() == 1 || (element.size() != element[lengthOffset] + 1))
                return infra::ConstByteRange();

            if (element[advertisingTypeOffset] == static_cast<uint8_t>(type))
                return infra::DiscardHead(element, headerSize);

            advData = infra::DiscardHead(advData, element.size());
        }

        return infra::ConstByteRange();
    }

    GapAdvertisementFormatter::GapAdvertisementFormatter(infra::BoundedVector<uint8_t>& payload)
        : payload(payload)
    {}

    void GapAdvertisementFormatter::AppendFlags(GapPeripheral::AdvertisementFlags flags)
    {
        really_assert(headerSize + sizeof(flags) <= RemainingSpaceAvailable());

        auto flagsByte = static_cast<uint8_t>(flags);
        AddHeader(payload, sizeof(flags), GapAdvertisementDataType::flags);
        AddData(payload, infra::MakeRangeFromSingleObject(flagsByte));
    }

    void GapAdvertisementFormatter::AppendCompleteLocalName(const infra::BoundedConstString& name)
    {
        really_assert(name.size() + headerSize <= RemainingSpaceAvailable() && name.size() > 0);

        AddHeader(payload, name.size(), GapAdvertisementDataType::completeLocalName);
        AddData(payload, infra::StringAsByteRange(name));
    }

    void GapAdvertisementFormatter::AppendShortenedLocalName(const infra::BoundedConstString& name)
    {
        really_assert(name.size() + headerSize <= RemainingSpaceAvailable() && name.size() > 0);

        AddHeader(payload, name.size(), GapAdvertisementDataType::shortenedLocalName);
        AddData(payload, infra::StringAsByteRange(name));
    }

    void GapAdvertisementFormatter::AppendManufacturerData(uint16_t manufacturerCode, infra::ConstByteRange data)
    {
        really_assert(data.size() + headerSize + sizeof(manufacturerCode) <= RemainingSpaceAvailable());

        AddHeader(payload, data.size() + sizeof(manufacturerCode), GapAdvertisementDataType::manufacturerSpecificData);
        AddData(payload, infra::ReinterpretCastMemoryRange<const uint8_t>(infra::MakeRangeFromSingleObject(manufacturerCode)));
        AddData(payload, data);
    }

    void GapAdvertisementFormatter::AppendListOfServicesUuid(infra::MemoryRange<AttAttribute::Uuid16> services)
    {
        really_assert(services.size() * sizeof(AttAttribute::Uuid16) + headerSize <= RemainingSpaceAvailable() && !services.empty());

        AddHeader(payload, services.size() * sizeof(AttAttribute::Uuid16), GapAdvertisementDataType::completeListOf16BitUuids);

        for (const auto& service : services)
            AddData(payload, infra::ReinterpretCastMemoryRange<const uint8_t>(infra::MakeRangeFromSingleObject(service)));
    }

    void GapAdvertisementFormatter::AppendListOfServicesUuid(infra::MemoryRange<AttAttribute::Uuid128> services)
    {
        really_assert(services.size() * sizeof(AttAttribute::Uuid128) + headerSize <= RemainingSpaceAvailable() && !services.empty());

        AddHeader(payload, services.size() * sizeof(AttAttribute::Uuid128), GapAdvertisementDataType::completeListOf128BitUuids);

        for (auto& service : services)
            AddData(payload, infra::ReinterpretCastMemoryRange<uint8_t>(infra::MakeRangeFromSingleObject(service)));
    }

    void GapAdvertisementFormatter::AppendPublicTargetAddress(hal::MacAddress address)
    {
        really_assert(sizeof(address) + headerSize <= RemainingSpaceAvailable());

        AddHeader(payload, sizeof(address), GapAdvertisementDataType::publicTargetAddress);
        AddData(payload, infra::ReinterpretCastMemoryRange<const uint8_t>(infra::MakeRangeFromSingleObject(address)));
    }

    void GapAdvertisementFormatter::AppendAppearance(uint16_t appearance)
    {
        really_assert(sizeof(appearance) + headerSize <= RemainingSpaceAvailable());

        AddHeader(payload, sizeof(appearance), GapAdvertisementDataType::appearance);
        AddData(payload, infra::ReinterpretCastMemoryRange<const uint8_t>(infra::MakeRangeFromSingleObject(appearance)));
    }

    infra::ConstByteRange GapAdvertisementFormatter::FormattedAdvertisementData() const
    {
        return infra::MakeRange(payload);
    }

    std::size_t GapAdvertisementFormatter::RemainingSpaceAvailable() const
    {
        return payload.max_size() - payload.size();
    }
}

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, const services::GapAdvertisingEventType& eventType)
    {
        if (eventType == services::GapAdvertisingEventType::advInd)
            stream << "ADV_IND";
        else if (eventType == services::GapAdvertisingEventType::advDirectInd)
            stream << "ADV_DIRECT_IND";
        else if (eventType == services::GapAdvertisingEventType::advScanInd)
            stream << "ADV_SCAN_IND";
        else if (eventType == services::GapAdvertisingEventType::scanResponse)
            stream << "SCAN_RESPONSE";
        else
            stream << "ADV_NONCONN_IND";

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream& stream, const services::GapDeviceAddressType& addressType)
    {
        if (addressType == services::GapDeviceAddressType::publicAddress)
            stream << "Public Device Address";
        else
            stream << "Random Device Address";

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream& stream, const services::GapState& state)
    {
        if (state == services::GapState::standby)
            stream << "Standby";
        else if (state == services::GapState::scanning)
            stream << "Scanning";
        else if (state == services::GapState::advertising)
            stream << "Advertising";
        else if (state == services::GapState::initiating)
            stream << "Initiating";
        else
            stream << "Connected";

        return stream;
    }
}
