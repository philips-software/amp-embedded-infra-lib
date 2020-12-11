#ifndef HAL_ASYNCH_GPIO_HPP
#define HAL_ASYNCH_GPIO_HPP

#include "hal/interfaces/Gpio.hpp"

namespace hal
{
    class AsyncGpioPin
    {
    protected:
        AsyncGpioPin() = default;
        AsyncGpioPin(const AsyncGpioPin& other) = delete;
        AsyncGpioPin& operator=(const AsyncGpioPin& other) = delete;
        ~AsyncGpioPin() = default;

    public:
        virtual void Get(const infra::Function<void(bool result)>& onDone) const = 0;
        virtual void Set(bool value, const infra::Function<void()>& onDone) = 0;
        virtual void GetOutputLatch(const infra::Function<void(bool result)>& onDone) const = 0;
        virtual void SetAsInput(const infra::Function<void()>& onDone) = 0;
        virtual void IsInput(const infra::Function<void(bool isInput)>& onDone) const = 0;

        virtual void Configure(PinConfigType config, const infra::Function<void()>& onDone) = 0;
        virtual void Configure(PinConfigType config, bool startOutputState, const infra::Function<void()>& onDone) = 0;
        virtual void Unconfigure(const infra::Function<void()>& onDone) = 0;

        virtual void EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, const infra::Function<void()>& onDone) = 0;
        virtual void DisableInterrupt(const infra::Function<void()>& onDone) = 0;
    };

    class AsyncInputPin
    {
    public:
        explicit AsyncInputPin(AsyncGpioPin& pin, const infra::Function<void()>& onDone);
        AsyncInputPin(const AsyncInputPin& other) = delete;
        AsyncInputPin& operator=(const AsyncInputPin& other) = delete;

        void Get(const infra::Function<void(bool result)>& onDone) const;

        void EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, const infra::Function<void()>& onDone);
        void DisableInterrupt(const infra::Function<void()>& onDone);

    private:
        AsyncGpioPin& pin;
    };

    class AsyncOutputPin
    {
    public:
        explicit AsyncOutputPin(AsyncGpioPin& pin, bool startState, const infra::Function<void()>& onDone);
        AsyncOutputPin(const AsyncOutputPin& other) = delete;
        AsyncOutputPin& operator=(const AsyncOutputPin& other) = delete;

        void Set(bool value, const infra::Function<void()>& onDone);
        void GetOutputLatch(const infra::Function<void(bool result)>& onDone) const;

    private:
        AsyncGpioPin& pin;
    };

    class AsyncTriStatePin
    {
    public:
        explicit AsyncTriStatePin(AsyncGpioPin& pin, const infra::Function<void()>& onDone);
        AsyncTriStatePin(AsyncGpioPin& pin, bool startOutputState, const infra::Function<void()>& onDone);
        AsyncTriStatePin(const AsyncTriStatePin& other) = delete;
        AsyncTriStatePin& operator=(const AsyncTriStatePin& other) = delete;

        void Get(const infra::Function<void(bool result)>& onDone) const;
        void GetOutputLatch(const infra::Function<void(bool result)>& onDone) const;
        void Set(bool value, const infra::Function<void()>& onDone);
        void SetAsInput(const infra::Function<void()>& onDone);
        void IsInput(const infra::Function<void(bool result)>& onDone) const;

        void EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, const infra::Function<void()>& onDone);
        void DisableInterrupt(const infra::Function<void()>& onDone);

    private:
        AsyncGpioPin& pin;
    };
}

#endif
