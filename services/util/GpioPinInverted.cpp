#include "services/util/GpioPinInverted.hpp"

namespace services
{
    GpioPinInverted::GpioPinInverted(hal::GpioPin& pin)
        : pin(pin)
    {}

    bool GpioPinInverted::Get() const
    {
        return !pin.Get();
    }

    void GpioPinInverted::Set(bool value)
    {
        pin.Set(!value);
    }

    bool GpioPinInverted::GetOutputLatch() const
    {
        return !pin.GetOutputLatch();
    }

    void GpioPinInverted::SetAsInput()
    {
        pin.SetAsInput();
    }

    bool GpioPinInverted::IsInput() const
    {
        return pin.IsInput();
    }

    void GpioPinInverted::Config(hal::PinConfigType config)
    {
        pin.Config(config);
    }

    void GpioPinInverted::Config(hal::PinConfigType config, bool startOutputState)
    {
        pin.Config(config, !startOutputState);
    }

    void GpioPinInverted::ResetConfig()
    {
        pin.ResetConfig();
    }

    void GpioPinInverted::EnableInterrupt(const infra::Function<void()>& action, hal::InterruptTrigger trigger)
    {
        static const std::array<hal::InterruptTrigger, 3> inverse = { hal::InterruptTrigger::fallingEdge, hal::InterruptTrigger::risingEdge,
            hal::InterruptTrigger::bothEdges };
        assert(static_cast<uint8_t>(trigger) < inverse.size());
        pin.EnableInterrupt(action, inverse[static_cast<uint8_t>(trigger)]);
    }

    void GpioPinInverted::DisableInterrupt()
    {
        pin.DisableInterrupt();
    }
}
