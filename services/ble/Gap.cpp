#include "services/ble/Gap.hpp"

namespace services
{
    void GapPeripheralDecorator::StateChanged(GapState state)
    {
        GapPeripheral::NotifyObservers([&state](auto& obs)
            { obs.StateChanged(state); });
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

    infra::ConstByteRange GapPeripheralDecorator::GetAdvertisementData()
    {
        return GapPeripheralObserver::Subject().GetAdvertisementData();
    }

    void GapPeripheralDecorator::SetScanResponseData(infra::ConstByteRange data)
    {
        GapPeripheralObserver::Subject().SetScanResponseData(data);
    }

    infra::ConstByteRange GapPeripheralDecorator::GetScanResponseData()
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
        GapCentralObserver::SubjectType::NotifyObservers([](auto& obs) { obs.AuthenticationSuccessfullyCompleted(); });
    }

    void GapCentralDecorator::AuthenticationFailed(GapAuthenticationErrorType status)
    {
        GapCentralObserver::SubjectType::NotifyObservers([&status](auto& obs) { obs.AuthenticationFailed(status); });
    }

    void GapCentralDecorator::DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered)
    {
        GapCentralObserver::SubjectType::NotifyObservers([&deviceDiscovered](auto& obs) { obs.DeviceDiscovered(deviceDiscovered); });
    }

    void GapCentralDecorator::StateChanged(GapState state)
    {
        GapCentralObserver::SubjectType::NotifyObservers([&state](auto& obs) { obs.StateChanged(state); });
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
}
