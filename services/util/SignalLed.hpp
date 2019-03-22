#ifndef SERVICES_SIGNAL_LED_HPP
#define SERVICES_SIGNAL_LED_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/timer/Timer.hpp"

namespace services
{
    class SignalLed
    {
    public:
        SignalLed(hal::GpioPin& led, uint32_t pattern, uint8_t length, infra::Duration duration);
        SignalLed(hal::GpioPin& led, infra::Duration duration);

        void Set(uint32_t pattern, uint8_t length);

    private:
        void OnTimer();

    private:
        hal::OutputPin led;
        uint32_t pattern = 0;
        uint8_t length = 0;
        infra::TimerRepeating timer;
        uint32_t index = 0;
    };
}

#endif
