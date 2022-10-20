#ifndef HAL_GPIO_MOCK_HPP
#define HAL_GPIO_MOCK_HPP

#include "hal/interfaces/Gpio.hpp"
#include "gmock/gmock.h"

namespace hal
{
    class GpioPinMock
        : public GpioPin
    {
    public:
        MOCK_CONST_METHOD0(Get, bool());
        MOCK_METHOD1(Set, void(bool value));
        MOCK_CONST_METHOD0(GetOutputLatch, bool());
        MOCK_METHOD0(SetAsInput, void());
        MOCK_CONST_METHOD0(IsInput, bool());
        MOCK_METHOD1(Config, void(PinConfigType config));
        MOCK_METHOD2(Config, void(PinConfigType config, bool startOutputState));
        MOCK_METHOD0(ResetConfig, void());
        MOCK_METHOD2(EnableInterrupt, void(const infra::Function<void()>& action, InterruptTrigger trigger));
        MOCK_METHOD0(DisableInterrupt, void());
    };
}

#endif
