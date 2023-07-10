#include "services/ble/Gap.hpp"

namespace services
{
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

    void GapCentralDecorator::AuthenticationSuccessfullyCompleted()
    {
        GapCentralObserver::SubjectType::NotifyObservers([](auto& obs)
            {
                obs.AuthenticationSuccessfullyCompleted();
            });
    }

    void GapCentralDecorator::AuthenticationFailed(GapAuthenticationErrorType status)
    {
        GapCentralObserver::SubjectType::NotifyObservers([&status](auto& obs)
            {
                obs.AuthenticationFailed(status);
            });
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
    { }

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
        const uint8_t payloadStartOffset = 2;
        infra::ConstByteRange searchRange = data;

        while (!searchRange.empty() && searchRange[lengthOffset] != 0)
        {
            auto advertisingType = searchRange[advertisingTypeOffset];
            auto payloadSize = searchRange[lengthOffset] + 1;
            infra::ConstByteRange payload(searchRange.begin() + payloadStartOffset, searchRange.begin() + payloadSize);
            searchRange.shrink_from_front_to(searchRange.size() - payloadSize);

            if (payload.size() > 1 && advertisingType == static_cast<uint8_t>(type))
                return payload;
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
