#ifndef SERVICES_DEBOUNCED_BUTTON_HPP
#define SERVICES_DEBOUNCED_BUTTON_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/timer/Timer.hpp"

namespace services
{
    class DebouncedButton
    {
    public:
        DebouncedButton(hal::GpioPin& buttonPin, infra::Function<void()> onPressed, infra::Function<void()> onReleased = infra::emptyFunction, infra::Duration debounceDuration = std::chrono::milliseconds(10));
        ~DebouncedButton();

    private:
        void ButtonChanged();
        void ButtonPressed();
        void ButtonReleased();
        void DebounceEnd() const;

    private:
        hal::InputPin buttonPin;
        infra::Duration debounceDuration;
        bool previousButtonState;
        infra::TimerSingleShot debounceEnd;
        infra::Function<void()> onPressed;
        infra::Function<void()> onReleased;
        bool announcedPressed = false;
    };
}

#endif
