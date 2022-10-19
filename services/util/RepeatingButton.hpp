#ifndef SERVICES_REPEATING_BUTTON_HPP
#define SERVICES_REPEATING_BUTTON_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/timer/Timer.hpp"

namespace services
{
    class RepeatingButton
    {
    public:
        struct Config
        {
            Config()
            {}

            infra::Duration initialDelay = std::chrono::milliseconds(500);
            infra::Duration successiveDelay = std::chrono::milliseconds(250);
            infra::Duration debounceDuration = std::chrono::milliseconds(10);
        };

        RepeatingButton(hal::GpioPin& buttonPin, infra::Function<void()> aCallback, const Config& config = Config());

        class FireTimer
            : public infra::Timer
        {
        public:
            explicit FireTimer(uint32_t timerServiceId = infra::systemTimerServiceId);

            void Start(infra::Duration initialDelay, infra::Duration successiveDelay, const infra::Function<void()>& action);

        protected:
            virtual void ComputeNextTriggerTime() override;

        private:
            infra::Duration initialDelay = infra::Duration();
            infra::Duration successiveDelay = infra::Duration();
            infra::TimePoint triggerStart;
        };

    private:
        void ButtonChanged();
        void ButtonPressed();
        void ButtonReleased();
        void DebounceEnd();

    private:
        hal::InputPin buttonPin;
        Config config;
        bool previousButtonState;
        FireTimer repeatingFireTimer;
        infra::TimerSingleShot debounceEnd;
        infra::Function<void()> callback;
    };
}

#endif
