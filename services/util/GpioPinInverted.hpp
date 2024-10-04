#ifndef SERVICES_GPIO_PIN_INVERTED_HPP
#define SERVICES_GPIO_PIN_INVERTED_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/util/Function.hpp"

namespace services
{
    class GpioPinInverted
        : public hal::GpioPin
    {
    public:
        explicit GpioPinInverted(hal::GpioPin& pin);

        bool Get() const override;
        void Set(bool value) override;
        bool GetOutputLatch() const override;
        void SetAsInput() override;
        bool IsInput() const override;
        void Config(hal::PinConfigType config) override;
        void Config(hal::PinConfigType config, bool startOutputState) override;
        void ResetConfig() override;
        void EnableInterrupt(const infra::Function<void()>& action, hal::InterruptTrigger trigger, hal::InterruptType type) override;
        void DisableInterrupt() override;

    private:
        hal::GpioPin& pin;
    };
}

#endif
