#include "services/ble/Gap.hpp"

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

    void GapPairingDecorator::Pair()
    {
        GapPairingObserver::Subject().Pair();
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

    void GapCentralDecorator::Connect(hal::MacAddress macAddress, GapDeviceAddressType addressType)
    {
        GapCentralObserver::Subject().Connect(macAddress, addressType);
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

    infra::ConstByteRange GapAdvertisingDataParser::ManufacturerSpecificData() const
    {
        return ParserAdvertisingData(GapAdvertisementDataType::manufacturerSpecificData);
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
                return infra::ConstByteRange(element.begin() + headerSize, element.begin() + element.size());

            advData = infra::DiscardHead(advData, element.size());
        }

        return infra::ConstByteRange();
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

    TextOutputStream& operator<<(TextOutputStream& stream, const services::GapAdvertisingEventAddressType& addressType)
    {
        if (addressType == services::GapAdvertisingEventAddressType::publicDeviceAddress)
            stream << "Public Device Address";
        else if (addressType == services::GapAdvertisingEventAddressType::randomDeviceAddress)
            stream << "Random Device Address";
        else if (addressType == services::GapAdvertisingEventAddressType::publicIdentityAddress)
            stream << "Public Identity Address";
        else
            stream << "Random Identity Address";

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
        else
            stream << "Connected";

        return stream;
    }
}
