#include "services/ble/Gap.hpp"

namespace services
{
    void GapPeripheralDecorator::StateUpdated(GapPeripheralState state)
    {
        GapPeripheralObserver::SubjectType::NotifyObservers([&state](auto& obs)
            { obs.StateUpdated(state); });
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

    void GapPeripheralDecorator::Advertise(AdvertisementType type, AdvertisementIntervalMultiplier multiplier)
    {
        GapPeripheralObserver::Subject().Advertise(type, multiplier);
    }

    void GapPeripheralDecorator::Standby()
    {
        GapPeripheralObserver::Subject().Standby();
    }
}
