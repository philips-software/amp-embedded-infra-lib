#include "services/ble/Gap.hpp"

namespace services
{
    void GapPeripheralDecorator::StateUpdated(GapState state)
    {
        GapPeripheralObserver::SubjectType::NotifyObservers([&state](auto& obs) { obs.StateUpdated(state); });
    }

    hal::MacAddress GapPeripheralDecorator::GetResolvableAddress() const
    {
        return GapPeripheralObserver::Subject().GetResolvableAddress();
    }

    void GapPeripheralDecorator::SetAdvertisementData(infra::ConstByteRange data)
    {
        GapPeripheralObserver::Subject().SetAdvertisementData(data);
    }

    void GapPeripheralDecorator::SetScanResponseData(infra::ConstByteRange data)
    {
        GapPeripheralObserver::Subject().SetScanResponseData(data);
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

    void GapCentralDecorator::StateUpdated(GapState state)
    {
        GapCentralObserver::SubjectType::NotifyObservers([&state](auto& obs) { obs.StateUpdated(state); });
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
