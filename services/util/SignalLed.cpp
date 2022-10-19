#include "services/util/SignalLed.hpp"

namespace services
{
    SignalLed::SignalLed(hal::GpioPin& led, uint32_t pattern, uint8_t length, infra::Duration duration)
        : led(led)
        , timer(duration, [this]()
              { OnTimer(); })
    {
        Set(pattern, length);
    }

    SignalLed::SignalLed(hal::GpioPin& led, infra::Duration duration)
        : SignalLed(led, 0, 0, duration)
    {}

    void SignalLed::Set(uint32_t pattern, uint8_t length)
    {
        this->pattern = pattern;
        this->length = length;

        index = length - 1;
        OnTimer();
    }

    void SignalLed::OnTimer()
    {
        if (++index == length)
            index = 0;

        led.Set(((1 << index) & pattern) != 0);
    }
}
