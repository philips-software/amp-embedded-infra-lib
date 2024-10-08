#ifndef HAL_GPIO_HPP
#define HAL_GPIO_HPP

#include "infra/util/Function.hpp"
#include <cstdint>

namespace hal
{
    enum class InterruptTrigger : uint8_t
    {
        risingEdge,
        fallingEdge,
        bothEdges
    };

    enum class InterruptType : uint8_t
    {
        dispatched,
        immediate
    };

    enum class PinConfigType : uint8_t
    {
        input,
        output,
        triState
    };

    class GpioPin
    {
    public:
        GpioPin() = default;
        GpioPin(const GpioPin& other) = delete;
        GpioPin& operator=(const GpioPin& other) = delete;

    protected:
        ~GpioPin() = default;

    public:
        virtual bool Get() const = 0;
        virtual void Set(bool value) = 0;
        virtual bool GetOutputLatch() const = 0;
        virtual void SetAsInput() = 0;
        virtual bool IsInput() const = 0;

        virtual void Config(PinConfigType config) = 0;
        virtual void Config(PinConfigType config, bool startOutputState) = 0;
        virtual void ResetConfig() = 0;

        virtual void EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type = InterruptType::dispatched) = 0;
        virtual void DisableInterrupt() = 0;
    };

    class InputPin
    {
    public:
        explicit InputPin(GpioPin& pin);
        InputPin(const InputPin& other) = delete;
        InputPin& operator=(const InputPin& other) = delete;
        ~InputPin();

        bool Get() const;

        void EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type = InterruptType::dispatched);
        void DisableInterrupt();

    private:
        GpioPin& pin;
    };

    class OutputPin
    {
    public:
        explicit OutputPin(GpioPin& pin, bool startState = false);
        OutputPin(const OutputPin& other) = delete;
        OutputPin& operator=(const OutputPin& other) = delete;
        ~OutputPin();

        void Set(bool value);
        bool GetOutputLatch() const;

    private:
        GpioPin& pin;
    };

    class TriStatePin
    {
    public:
        explicit TriStatePin(GpioPin& pin);
        TriStatePin(GpioPin& pin, bool startOutputState);
        TriStatePin(const TriStatePin& other) = delete;
        TriStatePin& operator=(const TriStatePin& other) = delete;
        ~TriStatePin();

        bool Get() const;
        bool GetOutputLatch() const;
        void Set(bool value);
        void SetAsInput();
        bool IsInput() const;

        void EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type = InterruptType::dispatched);
        void DisableInterrupt();

    private:
        GpioPin& pin;
    };

    class DummyPin
        : public GpioPin
    {
    public:
        bool Get() const override;
        bool GetOutputLatch() const override;
        void Set(bool value) override;
        void SetAsInput() override;
        bool IsInput() const override;
        void Config(PinConfigType config) override;
        void Config(PinConfigType config, bool startOutputState) override;
        void ResetConfig() override;
        void EnableInterrupt(const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type = InterruptType::dispatched) override;
        void DisableInterrupt() override;
    };

    extern DummyPin dummyPin;

    class ScopedHigh
    {
    public:
        explicit ScopedHigh(OutputPin& pin);
        ScopedHigh(const ScopedHigh& other) = delete;
        ScopedHigh& operator=(const ScopedHigh& other) = delete;
        ~ScopedHigh();

    private:
        OutputPin& pin;
    };

    class ScopedLow
    {
    public:
        explicit ScopedLow(OutputPin& pin);
        ScopedLow(const ScopedLow& other) = delete;
        ScopedLow& operator=(const ScopedLow& other) = delete;
        ~ScopedLow();

    private:
        OutputPin& pin;
    };
}

#endif
