#ifndef SYNCHRONOUS_HAL_SYNCHRONOUS_GPIO_HPP
#define SYNCHRONOUS_HAL_SYNCHRONOUS_GPIO_HPP

#include <array>
#include <cstdlib>

namespace hal
{
    class SynchronousOutputPin
    {
    public:
        SynchronousOutputPin() = default;
        SynchronousOutputPin(const SynchronousOutputPin& other) = delete;
        SynchronousOutputPin& operator=(const SynchronousOutputPin& other) = delete;

        virtual void Set(bool value) = 0;
        virtual bool GetOutputLatch() const = 0;

    protected:
        ~SynchronousOutputPin() = default;
    };

    class SynchronousInputPin
    {
    public:
        SynchronousInputPin() = default;
        SynchronousInputPin(const SynchronousInputPin& other) = delete;

        virtual bool Get() = 0;

    protected:
        ~SynchronousInputPin() = default;
    };
}

#endif
