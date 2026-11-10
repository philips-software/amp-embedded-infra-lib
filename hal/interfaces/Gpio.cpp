#include "hal/interfaces/Gpio.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/ReallyAssert.hpp"
#include <utility>

namespace hal
{
    DummyPin dummyPin;

    GpioPinOwner::GpioPinOwner(GpioPin& pin)
        : pin{ &pin }
    {}

    GpioPinOwner::GpioPinOwner(GpioPinOwner&& other) noexcept(true)
        : pin{ other.pin }
    {
        other.pin = nullptr;
    }

    GpioPinOwner& GpioPinOwner::operator=(GpioPinOwner&& other) noexcept(true)
    {
        if (this != &other)
            pin = std::exchange(other.pin, nullptr);

        return *this;
    }

    GpioPinOwner::~GpioPinOwner()
    {
        if (pin != nullptr)
        {
            pin->ResetConfig();
            pin = nullptr;
        }
    }

    GpioPin& GpioPinOwner::GetPin() const
    {
        really_assert(pin != nullptr);
        return *pin;
    }

    bool GpioPinOwner::HasPin() const
    {
        return pin != nullptr;
    }

    InputPin::InputPin(GpioPin& pin)
        : GpioPinOwner{ pin }
    {
        pin.Config(PinConfigType::input);
    }

    InputPin::InputPin(GpioPinOwner&& pin) noexcept(true)
        : GpioPinOwner{ std::move(pin) }
    {
        GetPin().ReConfig(PinConfigType::input);
    }

    InputPin::InputPin(InputPin&& other) noexcept(true)
        : GpioPinOwner{ std::move(other) }
    {
        /* we moved from an input pin, no reconfiguration needed */
    }

    bool InputPin::Get() const
    {
        return GetPin().Get();
    }

    void InputPin::EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type) const
    {
        GetPin().EnableInterrupt(action, trigger, type);
    }

    void InputPin::DisableInterrupt() const
    {
        GetPin().DisableInterrupt();
    }

    OutputPin::OutputPin(GpioPin& pin, bool startState)
        : GpioPinOwner{ pin }
    {
        pin.Config(PinConfigType::output, startState);
    }

    OutputPin::OutputPin(GpioPinOwner&& pin, bool startState) noexcept(true)
        : GpioPinOwner{ std::move(pin) }
    {
        GetPin().ReConfig(PinConfigType::output, startState);
    }

    OutputPin::OutputPin(OutputPin&& other, bool startState) noexcept(true)
        : GpioPinOwner{ std::move(other) }
    {
        /* we moved from an output pin, no reconfiguration needed */
        Set(startState);
    }

    void OutputPin::Set(bool value) const
    {
        GetPin().Set(value);
    }

    bool OutputPin::GetOutputLatch() const
    {
        return GetPin().GetOutputLatch();
    }

    TriStatePin::TriStatePin(GpioPin& pin)
        : GpioPinOwner{ pin }
    {
        pin.Config(PinConfigType::triState);
    }

    TriStatePin::TriStatePin(GpioPin& pin, bool startOutputState)
        : GpioPinOwner{ pin }
    {
        pin.Config(PinConfigType::triState, startOutputState);
    }

    TriStatePin::TriStatePin(GpioPinOwner&& pin) noexcept(true)
        : GpioPinOwner{ std::move(pin) }
    {
        GetPin().ReConfig(PinConfigType::triState);
    }

    TriStatePin::TriStatePin(TriStatePin&& other) noexcept(true)
        : GpioPinOwner{ std::move(other) }
    {
        /* we moved from a tri-state pin, no reconfiguration needed */
        SetAsInput();
    }

    TriStatePin::TriStatePin(GpioPinOwner&& pin, bool startState) noexcept(true)
        : GpioPinOwner{ std::move(pin) }
    {
        GetPin().ReConfig(PinConfigType::triState, startState);
    }

    TriStatePin::TriStatePin(TriStatePin&& other, bool startState) noexcept(true)
        : GpioPinOwner{ std::move(other) }
    {
        /* we moved from a tri-state pin, no reconfiguration needed */
        Set(startState);
    }

    bool TriStatePin::Get() const
    {
        return GetPin().Get();
    }

    void TriStatePin::Set(bool value) const
    {
        GetPin().Set(value);
    }

    bool TriStatePin::GetOutputLatch() const
    {
        return GetPin().GetOutputLatch();
    }

    void TriStatePin::SetAsInput() const
    {
        GetPin().SetAsInput();
    }

    bool TriStatePin::IsInput() const
    {
        return GetPin().IsInput();
    }

    void TriStatePin::EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type) const
    {
        GetPin().EnableInterrupt(action, trigger, type);
    }

    void TriStatePin::DisableInterrupt() const
    {
        GetPin().DisableInterrupt();
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
    {
        /* dummy pin, no behaviour */
    }

    void DummyPin::SetAsInput()
    {
        /* dummy pin, no behaviour */
    }

    bool DummyPin::IsInput() const
    {
        return false;
    }

    void DummyPin::Config(PinConfigType config)
    {
        /* dummy pin, no behaviour */
    }

    void DummyPin::Config(PinConfigType config, bool startOutputState)
    {
        /* dummy pin, no behaviour */
    }

    void DummyPin::ReConfig(PinConfigType config)
    {
        /* dummy pin, no behaviour */
    }

    void DummyPin::ReConfig(PinConfigType config, bool startOutputState)
    {
        /* dummy pin, no behaviour */
    }

    void DummyPin::ResetConfig()
    {
        /* dummy pin, no behaviour */
    }

    void DummyPin::EnableInterrupt(const infra::Function<void()>& actionOnInterrupt, InterruptTrigger trigger, InterruptType type)
    {
        /* dummy pin, no behaviour */
    }

    void DummyPin::DisableInterrupt()
    {
        /* dummy pin, no behaviour */
    }

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
