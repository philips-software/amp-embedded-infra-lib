#ifndef SERVICES_INPUT_PIN_MULTIPLE_ACCESS_HPP
#define SERVICES_INPUT_PIN_MULTIPLE_ACCESS_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/Observer.hpp"

namespace services
{
    class InputPinMultipleAccess
        : public hal::InputPin
        , public infra::Observer<InputPinMultipleAccess, class InputPinMultipleAccessMaster>
    {
    public:
        explicit InputPinMultipleAccess(InputPinMultipleAccessMaster& master);
        InputPinMultipleAccess(const InputPinMultipleAccess& other) = delete;
        InputPinMultipleAccess& operator=(const InputPinMultipleAccess& other) = delete;
        ~InputPinMultipleAccess();

        bool Get() const override;

        void EnableInterrupt(const infra::Function<void()>& action, hal::InterruptTrigger trigger, hal::InterruptType type = hal::InterruptType::dispatched) override;
        void DisableInterrupt() override;

        hal::InterruptTrigger GetInterruptTrigger();
        hal::InterruptType GetInterruptType();
        void OnInterrupt();

    private:
        InputPinMultipleAccessMaster& master;

        hal::InterruptTrigger trigger;
        hal::InterruptType type;
        infra::Function<void()> empty;
        infra::Function<void()>& action;
    };

    class InputPinMultipleAccessMaster
        : public hal::InputPin
        , public infra::Subject<InputPinMultipleAccess>
    {
    public:
        explicit InputPinMultipleAccessMaster(hal::GpioPin& pin);

        InputPinMultipleAccessMaster(const InputPinMultipleAccessMaster& other) = delete;
        InputPinMultipleAccessMaster& operator=(const InputPinMultipleAccessMaster& other) = delete;
        ~InputPinMultipleAccessMaster();

    private:
        infra::Function<void()> onInterrupt;
    };
}

#endif
