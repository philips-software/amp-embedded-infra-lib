#ifndef BLE_INTERFACES_HPP
#define BLE_INTERFACES_HPP

#include "infra/util/Observer.hpp"

namespace ble
{
    class GapPeripheralObserver;

    class GapPeripheral
        : public infra::Subject<GapPeripheralObserver>
    {
    public:
        enum class State
        {
            Standby,
            Advertising,
            Connected
        };

        virtual void Advertise() = 0;
        virtual void Standby() = 0;
    };

    class GapPeripheralObserver
        : public infra::SingleObserver<GapPeripheralObserver, GapPeripheral>
    {
    public:
        virtual void State(GapPeripheral::State state) = 0;
    };
}

#endif
