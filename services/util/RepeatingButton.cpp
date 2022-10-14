#include "services/util/RepeatingButton.hpp"

namespace services
{

    RepeatingButton::RepeatingButton(hal::GpioPin& buttonPin, infra::Function<void()> aCallback, const Config& config)
        : buttonPin(buttonPin)
        , config(config)
        , previousButtonState(false)
        , callback(aCallback)
    {
        this->buttonPin.EnableInterrupt([this]()
            { ButtonChanged(); },
            hal::InterruptTrigger::bothEdges);
    }

    void RepeatingButton::ButtonChanged()
    {
        bool buttonState = buttonPin.Get();

        if (previousButtonState != buttonState)
        {
            if (buttonState)
                ButtonPressed();
            else
                ButtonReleased();
        }

        previousButtonState = buttonState;
    }

    void RepeatingButton::ButtonPressed()
    {
        if (!debounceEnd.Armed())
        {
            debounceEnd.Start(config.debounceDuration, [this]()
                { DebounceEnd(); });
            repeatingFireTimer.Start(config.initialDelay, config.successiveDelay, [this]()
                { callback(); });
        }
    }

    void RepeatingButton::ButtonReleased()
    {
        if (!debounceEnd.Armed())
            repeatingFireTimer.Cancel();
    }

    void RepeatingButton::DebounceEnd()
    {
        if (!buttonPin.Get())
            repeatingFireTimer.Cancel();
    }

    RepeatingButton::FireTimer::FireTimer(uint32_t timerServiceId)
        : infra::Timer(timerServiceId)
    {}

    void RepeatingButton::FireTimer::Start(infra::Duration initialDelay, infra::Duration successiveDelay, const infra::Function<void()>& action)
    {
        this->initialDelay = initialDelay;
        this->successiveDelay = successiveDelay;
        SetNextTriggerTime(Now() + initialDelay, action);
        Action()();
    }

    void RepeatingButton::FireTimer::ComputeNextTriggerTime()
    {
        SetNextTriggerTime(Now() + successiveDelay, Action());
    }
}
