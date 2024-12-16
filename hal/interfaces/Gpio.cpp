#include "hal/interfaces/Gpio.hpp"
#include "infra/util/Function.hpp"

namespace hal
{
    DummyPin dummyPin;

    InputPin::InputPin(GpioPin& pin)
        : pin(pin)
    {
        pin.Config(PinConfigType::input);
    }

    InputPin::~InputPin()
    {
        pin.ResetConfig();
    }

    bool InputPin::Get() const
    {
        return pin.Get();
    }

    void InputPin::EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type)
    {
        pin.EnableInterrupt(action, trigger, type);
    }

    void InputPin::DisableInterrupt()
    {
        pin.DisableInterrupt();
    }

    OutputPin::OutputPin(GpioPin& pin, bool startState)
        : pin(pin)
    {
        pin.Config(PinConfigType::output, startState);
    }

    OutputPin::~OutputPin()
    {
        pin.ResetConfig();
    }

    void OutputPin::Set(bool value)
    {
        pin.Set(value);
    }

    bool OutputPin::GetOutputLatch() const
    {
        return pin.GetOutputLatch();
    }

    TriStatePin::TriStatePin(GpioPin& pin)
        : pin(pin)
    {
        pin.Config(PinConfigType::triState);
    }

    TriStatePin::TriStatePin(GpioPin& pin, bool startOutputState)
        : pin(pin)
    {
        pin.Config(PinConfigType::triState, startOutputState);
    }

    TriStatePin::~TriStatePin()
    {
        pin.ResetConfig();
    }

    bool TriStatePin::Get() const
    {
        return pin.Get();
    }

    void TriStatePin::Set(bool value)
    {
        pin.Set(value);
    }

    bool TriStatePin::GetOutputLatch() const
    {
        return pin.GetOutputLatch();
    }

    void TriStatePin::SetAsInput()
    {
        pin.SetAsInput();
    }

    bool TriStatePin::IsInput() const
    {
        return pin.IsInput();
    }

    void TriStatePin::EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type)
    {
        pin.EnableInterrupt(action, trigger, type);
    }

    void TriStatePin::DisableInterrupt()
    {
        pin.DisableInterrupt();
    }

    bool DummyPin::Get() const
    {
        return false;
    }

    bool DummyPin::GetOutputLatch() const
    {
        return false;
    }

    void DummyPin::Set(bool value)
    {}

    void DummyPin::SetAsInput()
    {}

    bool DummyPin::IsInput() const
    {
        return false;
    }

    void DummyPin::Config(PinConfigType config)
    {}

    void DummyPin::Config(PinConfigType config, bool startOutputState)
    {}

    void DummyPin::ResetConfig()
    {}

    void DummyPin::EnableInterrupt(const infra::Function<void()>& actionOnInterrupt, InterruptTrigger trigger, InterruptType type)
    {}

    void DummyPin::DisableInterrupt()
    {}

    ScopedHigh::ScopedHigh(OutputPin& pin)
        : pin(pin)
    {
        pin.Set(true);
    }

    ScopedHigh::~ScopedHigh()
    {
        pin.Set(false);
    }

    ScopedLow::ScopedLow(OutputPin& pin)
        : pin(pin)
    {
        pin.Set(false);
    }

    ScopedLow::~ScopedLow()
    {
        pin.Set(true);
    }
}
