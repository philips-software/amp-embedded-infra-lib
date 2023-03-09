#include "services/ble/Gap.hpp"

namespace services
{
    void GapPeripheralDecorator::StateChanged(GapPeripheralState state)
    {
        GapPeripheralObserver::SubjectType::NotifyObservers([&state](auto& obs) { obs.StateChanged(state); });
    }

    GapAddress GapPeripheralDecorator::GetAddress() const
    {
        return GapPeripheralObserver::Subject().GetAddress();
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
}
