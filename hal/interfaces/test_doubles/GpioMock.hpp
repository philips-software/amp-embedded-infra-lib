#ifndef HAL_GPIO_MOCK_HPP
#define HAL_GPIO_MOCK_HPP

#include "hal/interfaces/Gpio.hpp"
#include "infra/util/Function.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class GpioPinMock
        : public GpioPin
    {
    public:
        MOCK_METHOD(bool, Get, (), (const override));
        MOCK_METHOD(void, Set, (bool value), (override));
        MOCK_METHOD(bool, GetOutputLatch, (), (const override));
        MOCK_METHOD(void, SetAsInput, (), (override));
        MOCK_METHOD(bool, IsInput, (), (const override));
        MOCK_METHOD(void, Config, (PinConfigType config), (override));
        MOCK_METHOD(void, Config, (PinConfigType config, bool startOutputState), (override));
        MOCK_METHOD(void, ResetConfig, (), (override));
        MOCK_METHOD(void, EnableInterrupt, (const infra::Function<void()>& action, InterruptTrigger trigger, InterruptType type), (override));
        MOCK_METHOD(void, DisableInterrupt, (), (override));
    };
}

#endif
