#include "services/ble/GapPeripheralIntervalDecorator.hpp"
#include <cassert>

namespace services
{
    const services::GapConnectionParameters GapPeripheralIntervalDecorator::defaultConnParam{
        6,
        6,
        0,
        500,
    };

    const services::GapConnectionParameters GapPeripheralIntervalDecorator::longConnParam{
        50,
        50,
        0,
        500,
    };

    GapPeripheralIntervalDecorator::GapPeripheralIntervalDecorator(GapPeripheral& subject)
        : GapPeripheralDecorator(subject)
    {}

    void GapPeripheralIntervalDecorator::SwitchToLongInterval()
    {
        useLongInterval = true;
        SwitchInterval();
    }

    void GapPeripheralIntervalDecorator::SwitchToUserInterval()
    {
        useLongInterval = false;
        SwitchInterval();
    }

    void GapPeripheralIntervalDecorator::Advertise(services::GapAdvertisementType type, AdvertisementIntervalMultiplier multiplier)
    {
        userAdvparam.emplace(type, multiplier);
        GapPeripheralDecorator::Advertise(type, useLongInterval ? longAdvMutiplier : multiplier);
    }

    void GapPeripheralIntervalDecorator::Standby()
    {
        pendingAdv = false;
        GapPeripheralDecorator::Standby();
    }

    void GapPeripheralIntervalDecorator::StateChanged(services::GapState state)
    {
        this->state = state;
        GapPeripheralDecorator::StateChanged(state);
        if (state == services::GapState::connected)
            UpdateConnectionParameters();
        else if (state == services::GapState::standby)
            HandlePendingAdvertise();
    }

    void GapPeripheralIntervalDecorator::SetConnectionParameters(const services::GapConnectionParameters& connParam)
    {
        userConnParam = connParam;
        if (state == services::GapState::connected)
            UpdateConnectionParameters();
    }

    void GapPeripheralIntervalDecorator::SwitchInterval()
    {
        if (state == services::GapState::advertising)
        {
            assert(userAdvparam.has_value());
            Standby();
            pendingAdv = true;
        }
        else if (state == services::GapState::connected)
            UpdateConnectionParameters();
    }

    void GapPeripheralIntervalDecorator::UpdateConnectionParameters()
    {
        if (useLongInterval)
            GapPeripheralDecorator::SetConnectionParameters(longConnParam);
        else
            GapPeripheralDecorator::SetConnectionParameters(userConnParam.has_value() ? userConnParam.value() : defaultConnParam);
    }

    void GapPeripheralIntervalDecorator::HandlePendingAdvertise()
    {
        if (pendingAdv)
        {
            pendingAdv = false;
            auto [userAdvType, userAdvMultiplier] = userAdvparam.value();
            GapPeripheralDecorator::Advertise(userAdvType, useLongInterval ? longAdvMutiplier : userAdvMultiplier);
        }
    }
}
