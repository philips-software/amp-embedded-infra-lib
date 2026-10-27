#include "services/ble/Gap.hpp"
#include "infra/stream/ByteInputStream.hpp"
#include "infra/util/BoundedString.hpp"
#include "infra/util/LogAndAbort.hpp"
#include "infra/util/MemoryRange.hpp"
#include <algorithm>
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

    bool ListContainsUuid(infra::ConstByteRange list, infra::ConstByteRange uuid)
    {
        if (list.size() % uuid.size() != 0)
            return false;

        for (; !list.empty(); list = infra::DiscardHead(list, uuid.size()))
            if (std::equal(uuid.begin(), uuid.end(), list.begin()))
                return true;

        return false;
    }
}

namespace services
{
    void GapPairingDecorator::AuthenticationRequired(bool isNumericComparison, uint32_t passkey)
    {
        GapPairing::NotifyObservers([&passkey, &isNumericComparison](auto& obs)
            {
                obs.AuthenticationRequired(isNumericComparison, passkey);
            });
    }

    void GapPairingDecorator::PairingResult(bool pairedSuccessfully, PairingFailedReason pairingFailedReason)
    {
        GapPairing::NotifyObservers([&pairedSuccessfully, &pairingFailedReason](auto& obs)
            {
                obs.PairingResult(pairedSuccessfully, pairingFailedReason);
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

    void GapBondingDecorator::RemoveBondWithAddress(GapAddress gapAddress)
    {
        GapBondingObserver::Subject().RemoveBondWithAddress(gapAddress);
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

    infra::MemoryRange<const services::Bond> GapBondingDecorator::GetBondList() const
    {
        return GapBondingObserver::Subject().GetBondList();
    }

    bool GapBondingDecorator::IsBondListPersisted() const
    {
        return GapBondingObserver::Subject().IsBondListPersisted();
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

    void GapPeripheralDecorator::SetIdentityAddress(GapAddress address)
    {
        GapPeripheralObserver::Subject().SetIdentityAddress(address);
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

    void GapPeripheralDecorator::SetDeviceName(infra::BoundedConstString name)
    {
        GapPeripheralObserver::Subject().SetDeviceName(name);
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

    void GapCentralDecorator::Standby()
    {
        GapCentralObserver::Subject().Standby();
    }

    void GapCentralDecorator::SetIdentityAddress(GapAddress address)
    {
        GapCentralObserver::Subject().SetIdentityAddress(address);
    }

    void GapCentralDecorator::StartDeviceDiscovery()
    {
        GapCentralObserver::Subject().StartDeviceDiscovery();
    }

    std::optional<hal::MacAddress> GapCentralDecorator::ResolvePrivateAddress(hal::MacAddress address) const
    {
        return GapCentralObserver::Subject().ResolvePrivateAddress(address);
    }

    void GapCentralDecorator::SetPrivacyMode(bool enabled)
    {
        GapCentralObserver::Subject().SetPrivacyMode(enabled);
    }

    void GapCentralDecorator::SetDeviceName(infra::BoundedConstString name)
    {
        GapCentralObserver::Subject().SetDeviceName(name);
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

    bool GapAdvertisingDataParser::ContainsServiceUuid(infra::ConstByteRange uuid) const
    {
        GapAdvertisementDataType incompleteListType;
        GapAdvertisementDataType completeListType;

        switch (uuid.size())
        {
            case sizeof(AttAttribute::Uuid16):
                incompleteListType = GapAdvertisementDataType::incompleteListOf16BitUuids;
                completeListType = GapAdvertisementDataType::completeListOf16BitUuids;
                break;
            case sizeof(uint32_t):
                incompleteListType = GapAdvertisementDataType::incompleteListOf32BitUuids;
                completeListType = GapAdvertisementDataType::completeListOf32BitUuids;
                break;
            case sizeof(AttAttribute::Uuid128):
                incompleteListType = GapAdvertisementDataType::incompleteListOf128BitUuids;
                completeListType = GapAdvertisementDataType::completeListOf128BitUuids;
                break;
            default:
                return false;
        }

        return ListContainsUuid(ParserAdvertisingData(incompleteListType), uuid) || ListContainsUuid(ParserAdvertisingData(completeListType), uuid);
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

    void GapAdvertisementFormatter::AppendListOfServicesUuid(GapAdvertisementDataType listType, infra::ConstByteRange uuids)
    {
        std::size_t uuidSize = 0;

        switch (listType)
        {
            case GapAdvertisementDataType::incompleteListOf16BitUuids:
            case GapAdvertisementDataType::completeListOf16BitUuids:
                uuidSize = sizeof(AttAttribute::Uuid16);
                break;
            case GapAdvertisementDataType::incompleteListOf32BitUuids:
            case GapAdvertisementDataType::completeListOf32BitUuids:
                uuidSize = sizeof(uint32_t);
                break;
            case GapAdvertisementDataType::incompleteListOf128BitUuids:
            case GapAdvertisementDataType::completeListOf128BitUuids:
                uuidSize = sizeof(AttAttribute::Uuid128);
                break;
            default:
                LOG_AND_ABORT_ENUM(listType);
        }

        really_assert(!uuids.empty() && uuids.size() % uuidSize == 0 && uuids.size() + headerSize <= RemainingSpaceAvailable());

        AddHeader(payload, uuids.size(), listType);
        AddData(payload, uuids);
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

    GapDeviceDiscoveryFilter::GapDeviceDiscoveryFilter(Type type, infra::ConstByteRange data)
        : type(type)
    {
        if (type == Type::none)
            return;

        really_assert(data.size() <= this->data.max_size());

        if (type == Type::manufacturerSpecificData)
            really_assert(data.size() >= sizeof(uint16_t));

        if (type == Type::serviceUuid)
            really_assert_with_msg(data.size() == sizeof(AttAttribute::Uuid16) || data.size() == sizeof(uint32_t) || data.size() == sizeof(AttAttribute::Uuid128), "Unexpected serviceUuid size: %d", static_cast<int>(data.size()));

        this->data.assign(data);
    }

    bool GapDeviceDiscoveryFilter::Passes(const GapAdvertisingReport& report) const
    {
        switch (type)
        {
            case Type::none:
                return true;
            case Type::address:
                return infra::ContentsEqual(infra::MakeRange(report.gapAddress.address), infra::MakeRange(data));
            case Type::manufacturerSpecificData:
            {
                auto manufacturerSpecificData = GapAdvertisingDataParser(report.data.range()).ManufacturerSpecificData();

                if (!manufacturerSpecificData)
                    return false;

                const uint16_t expectedManufacturerId = static_cast<uint16_t>(data[0] | (data[1] << 8));

                return expectedManufacturerId == manufacturerSpecificData->first && infra::ContentsEqual(infra::DiscardHead(infra::MakeRange(data), sizeof(uint16_t)), manufacturerSpecificData->second);
            }
            case Type::serviceUuid:
                return GapAdvertisingDataParser(report.data.range()).ContainsServiceUuid(infra::MakeRange(data));
        }

        return false;
    }
}

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, const services::AdvertisingReportType& reportType)
    {
        if (reportType == services::AdvertisingReportType::advInd)
            stream << "ADV_IND";
        else if (reportType == services::AdvertisingReportType::advDirectInd)
            stream << "ADV_DIRECT_IND";
        else if (reportType == services::AdvertisingReportType::advScanInd)
            stream << "ADV_SCAN_IND";
        else if (reportType == services::AdvertisingReportType::scanResponse)
            stream << "SCAN_RESPONSE";
        else if (reportType == services::AdvertisingReportType::advNonconnInd)
            stream << "ADV_NONCONN_IND";
        else
            LOG_AND_ABORT_ENUM(reportType);

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream& stream, const services::GapDeviceAddressType& addressType)
    {
        if (addressType == services::GapDeviceAddressType::publicAddress)
            stream << "Public";
        else if (addressType == services::GapDeviceAddressType::randomAddress)
            stream << "Random";
        else
            LOG_AND_ABORT_ENUM(addressType);

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
        else if (state == services::GapState::connected)
            stream << "Connected";
        else
            LOG_AND_ABORT_ENUM(state);

        return stream;
    }
}
