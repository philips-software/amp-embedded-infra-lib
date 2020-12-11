#include "hal/interfaces/AsyncGpio.hpp"

namespace hal
{
    AsyncInputPin::AsyncInputPin(AsyncGpioPin& pin, const infra::Function<void()>& onDone)
        : pin(pin)
    {
        pin.Configure(PinConfigType::input, onDone);
    }

    void AsyncInputPin::Get(const infra::Function<void(bool result)>& onDone) const
    {
        pin.Get(onDone);
    }

    void AsyncInputPin::EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, const infra::Function<void()>& onDone)
    {
        pin.EnableInterrupt(action, trigger, onDone);
    }

    void AsyncInputPin::DisableInterrupt(const infra::Function<void()>& onDone)
    {
        pin.DisableInterrupt(onDone);
    }

    AsyncOutputPin::AsyncOutputPin(AsyncGpioPin& pin, bool startState, const infra::Function<void()>& onDone)
        : pin(pin)
    {
        pin.Configure(PinConfigType::output, startState, onDone);
    }

    void AsyncOutputPin::Set(bool value, const infra::Function<void()>& onDone)
    {
        pin.Set(value, onDone);
    }

    void AsyncOutputPin::GetOutputLatch(const infra::Function<void(bool result)>& onDone) const
    {
        pin.GetOutputLatch(onDone);
    }

    AsyncTriStatePin::AsyncTriStatePin(AsyncGpioPin& pin, const infra::Function<void()>& onDone)
        : pin(pin)
    {
        pin.Configure(PinConfigType::triState, onDone);
    }

    AsyncTriStatePin::AsyncTriStatePin(AsyncGpioPin& pin, bool startOutputState, const infra::Function<void()>& onDone)
        : pin(pin)
    {
        pin.Configure(PinConfigType::triState, startOutputState, onDone);
    }

    void AsyncTriStatePin::Get(const infra::Function<void(bool result)>& onDone) const
    {
        pin.Get(onDone);
    }

    void AsyncTriStatePin::Set(bool value, const infra::Function<void()>& onDone)
    {
        pin.Set(value, onDone);
    }

    void AsyncTriStatePin::GetOutputLatch(const infra::Function<void(bool result)>& onDone) const
    {
        pin.GetOutputLatch(onDone);
    }

    void AsyncTriStatePin::SetAsInput(const infra::Function<void()>& onDone)
    {
        pin.SetAsInput(onDone);
    }

    void AsyncTriStatePin::IsInput(const infra::Function<void(bool result)>& onDone) const
    {
        pin.IsInput(onDone);
    }

    void AsyncTriStatePin::EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, const infra::Function<void()>& onDone)
    {
        pin.EnableInterrupt(action, trigger, onDone);
    }

    void AsyncTriStatePin::DisableInterrupt(const infra::Function<void()>& onDone)
    {
        pin.DisableInterrupt(onDone);
    }
}
