#ifndef BLE_INTERFACES_HPP
#define BLE_INTERFACES_HPP

#include "infra/util/Observer.hpp"
#include "hal/interfaces/MacAddress.hpp"

namespace ble
{
    constexpr uint8_t maxAdvertisementSize = 31;

    class GapPeripheral;

    enum class GapPeripheralState
    {
        Standby,
        Advertising,
        Connected
    };

    class GapPeripheralObserver
        : public infra::SingleObserver<GapPeripheralObserver, GapPeripheral>
    {
    public:
        using infra::SingleObserver<GapPeripheralObserver, GapPeripheral>::SingleObserver;

        virtual void State(GapPeripheralState state) = 0;
    };

    class GapPeripheral
        : public infra::Subject<GapPeripheralObserver>
    {
    public:
        virtual void SetPublicAddress(hal::MacAddress address) = 0;
        virtual hal::MacAddress GetPublicAddress() = 0;
        virtual void SetAdvertisementData(infra::ByteRange data) = 0;
        virtual void Advertise() = 0;
        virtual void Standby() = 0;
    };
}

#endif
