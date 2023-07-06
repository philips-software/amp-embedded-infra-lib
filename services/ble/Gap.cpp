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
}
