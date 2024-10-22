#include "hal/interfaces/Gpio.hpp"
#include "hal/interfaces/test_doubles/GpioMock.hpp"
#include "gtest/gtest.h"

TEST(GpioTest, InputPin)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::input));
    hal::InputPin inputPin(pin);

    EXPECT_CALL(pin, Get()).WillOnce(testing::Return(true));
    EXPECT_TRUE(inputPin.Get());

    EXPECT_CALL(pin, EnableInterrupt(testing::_, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched));
    inputPin.EnableInterrupt([]() {}, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched);

    EXPECT_CALL(pin, DisableInterrupt());
    inputPin.DisableInterrupt();

    EXPECT_CALL(pin, ResetConfig());
}

TEST(GpioTest, OutputPin)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::output, true));
    hal::OutputPin outputPin(pin, true);

    EXPECT_CALL(pin, Set(true));
    outputPin.Set(true);

    EXPECT_CALL(pin, GetOutputLatch()).WillOnce(testing::Return(true));
    EXPECT_TRUE(outputPin.GetOutputLatch());

    EXPECT_CALL(pin, ResetConfig());
}

TEST(GpioTest, TriStatePin_as_input)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::triState));
    hal::TriStatePin inputPin(pin);

    EXPECT_CALL(pin, IsInput()).WillOnce(testing::Return(true));
    EXPECT_TRUE(inputPin.IsInput());

    EXPECT_CALL(pin, Get()).WillOnce(testing::Return(true));
    EXPECT_TRUE(inputPin.Get());

    EXPECT_CALL(pin, EnableInterrupt(testing::_, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched));
    inputPin.EnableInterrupt([]() {}, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched);

    EXPECT_CALL(pin, DisableInterrupt());
    inputPin.DisableInterrupt();

    EXPECT_CALL(pin, ResetConfig());
}

TEST(GpioTest, TriStatePin_as_output)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::triState, true));
    hal::TriStatePin outputPin(pin, true);

    EXPECT_CALL(pin, Set(true));
    outputPin.Set(true);

    EXPECT_CALL(pin, GetOutputLatch()).WillOnce(testing::Return(true));
    EXPECT_TRUE(outputPin.GetOutputLatch());

    EXPECT_CALL(pin, SetAsInput());
    outputPin.SetAsInput();

    EXPECT_CALL(pin, ResetConfig());
}

TEST(GpioTest, DummyPin)
{
    hal::DummyPin pin;

    EXPECT_FALSE(pin.Get());
    EXPECT_FALSE(pin.GetOutputLatch());

    pin.Set(true);
    pin.SetAsInput();
    EXPECT_FALSE(pin.IsInput());

    pin.Config(hal::PinConfigType::input);
    pin.Config(hal::PinConfigType::output, true);
    pin.ResetConfig();

    pin.EnableInterrupt([]() {}, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched);
    pin.DisableInterrupt();
}

TEST(GpioTest, ScopedHigh)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::output, true));
    hal::OutputPin outputPin(pin, true);

    {
        EXPECT_CALL(pin, Set(true));
        hal::ScopedHigh scoped(outputPin);
        EXPECT_CALL(pin, Set(false));
    }

    EXPECT_CALL(pin, ResetConfig());
}

TEST(GpioTest, ScopedLow)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::output, true));
    hal::OutputPin outputPin(pin, true);

    {
        EXPECT_CALL(pin, Set(false));
        hal::ScopedLow scoped(outputPin);
        EXPECT_CALL(pin, Set(true));
    }

    EXPECT_CALL(pin, ResetConfig());
}
