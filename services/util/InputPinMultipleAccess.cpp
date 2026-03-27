#include "services/util/InputPinMultipleAccess.hpp"
#include "hal/interfaces/Gpio.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/Function.hpp"

namespace services
{
    InputPinMultipleAccess::InputPinMultipleAccess(InputPinMultipleAccessMaster& master)
        : hal::InputPin(hal::dummyPin)
        , master(master)
        , action(empty)
    {}

    InputPinMultipleAccess::~InputPinMultipleAccess()
    {
        if (Attached())
        {
            Detach();
        }
    }

    bool InputPinMultipleAccess::Get() const
    {
        return master.Get();
    }

    void InputPinMultipleAccess::EnableInterrupt(const infra::Function<void()>& action, hal::InterruptTrigger trigger, hal::InterruptType type)
    {
        this->action = action;
        this->trigger = trigger;
        this->type = type;
        Attach(master);
    }

    void InputPinMultipleAccess::DisableInterrupt()
    {
        if (Attached())
        {
            Detach();
        }
    }

    hal::InterruptTrigger InputPinMultipleAccess::GetInterruptTrigger()
    {
        return trigger;
    }

    hal::InterruptType InputPinMultipleAccess::GetInterruptType()
    {
        return type;
    }

    void InputPinMultipleAccess::OnInterrupt()
    {
        action();
    }

    InputPinMultipleAccessMaster::InputPinMultipleAccessMaster(hal::GpioPin& pin)
        : hal::InputPin(pin)
    {
        onInterrupt = [this, &pin]
        {
            NotifyObservers([&pin](auto& observer)
                {
                    auto actualTrigger = pin.Get() ? hal::InterruptTrigger::risingEdge : hal::InterruptTrigger::fallingEdge;
                    hal::InterruptTrigger requiredTrigger = observer.GetInterruptTrigger();

                    if (actualTrigger != requiredTrigger && requiredTrigger != hal::InterruptTrigger::bothEdges)
                        return;

                    if (observer.GetInterruptType() == hal::InterruptType::immediate)
                    {
                        observer.OnInterrupt();
                        return;
                    }

                    infra::EventDispatcher::Instance().Schedule([&observer]
                        {
                            observer.OnInterrupt();
                        });
                });
        };
        InputPin::EnableInterrupt(onInterrupt, hal::InterruptTrigger::bothEdges, hal::InterruptType::immediate);
    }

    InputPinMultipleAccessMaster::~InputPinMultipleAccessMaster()
    {
        InputPin::DisableInterrupt();
    }
}
