#ifndef SERVICES_GPIO_PIN_INVERTED_HPP
#define SERVICES_GPIO_PIN_INVERTED_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/timer/Timer.hpp"

namespace services
{
    class GpioPinInverted
        : public hal::GpioPin
    {
    public:
        explicit GpioPinInverted(hal::GpioPin& pin);

        virtual bool Get() const override;
        virtual void Set(bool value) override;
        virtual bool GetOutputLatch() const override;
        virtual void SetAsInput() override;
        virtual bool IsInput() const override;
        virtual void Config(hal::PinConfigType config) override;
        virtual void Config(hal::PinConfigType config, bool startOutputState) override;
        virtual void ResetConfig() override;
        virtual void EnableInterrupt(const infra::Function<void()>& action, hal::InterruptTrigger trigger) override;
        virtual void DisableInterrupt() override;

    private:
        hal::GpioPin& pin;
    };
}

#endif
