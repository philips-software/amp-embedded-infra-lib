#ifndef SERVICES_DEBUG_LED_HPP
#define SERVICES_DEBUG_LED_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/timer/TimerAlternating.hpp"

namespace services
{
    class DebugLed
    {
    public:
        DebugLed(hal::GpioPin& pin, infra::Duration onDuration = std::chrono::milliseconds(200),
            infra::Duration offDuration = std::chrono::milliseconds(800), uint32_t timerServiceId = infra::systemTimerServiceId);

    private:
        hal::OutputPin pin;
        infra::TimerAlternating debugLedTimer;
    };
}

#endif
