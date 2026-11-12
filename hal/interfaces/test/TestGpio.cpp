#include "hal/interfaces/Gpio.hpp"
#include "hal/interfaces/test_doubles/GpioMock.hpp"
#include "infra/util/Function.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>
#include <utility>

struct GpioPinOwnerSpy : hal::GpioPinOwner
{
    using GpioPinOwner::GetPin;
    using GpioPinOwner::GpioPinOwner;
};

TEST(GpioTest, GpioPinOwner)
{
    testing::StrictMock<hal::GpioPinMock> pin1;
    testing::StrictMock<hal::GpioPinMock> pin2;

    GpioPinOwnerSpy owner1{ pin1 };
    GpioPinOwnerSpy owner2{ pin2 };

    {
        EXPECT_CALL(pin2, ResetConfig());
        owner2 = std::move(owner1);
    }

    EXPECT_CALL(pin1, Set(true));
    owner2.GetPin().Set(true);

    EXPECT_CALL(pin1, ResetConfig());
}

TEST(GpioTest, InputPin)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::input));
    hal::InputPin inputPin(pin);

    EXPECT_CALL(pin, Get()).WillOnce(testing::Return(true));
    EXPECT_TRUE(inputPin.Get());

    EXPECT_CALL(pin, EnableInterrupt(testing::_, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched));
    inputPin.EnableInterrupt(infra::emptyFunction, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched);

    EXPECT_CALL(pin, DisableInterrupt());
    inputPin.DisableInterrupt();

    EXPECT_CALL(pin, ResetConfig());
}

TEST(GpioTest, MoveInputPinToInputPin)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::input)).Times(1);
    std::unique_ptr<hal::InputPin> movedFromPin{ std::make_unique<hal::InputPin>(pin) };
    std::unique_ptr<hal::InputPin> movedToPin{ std::make_unique<hal::InputPin>(std::move(*movedFromPin)) };

    movedFromPin = nullptr;

    EXPECT_CALL(pin, ResetConfig()).Times(1);
    movedToPin = nullptr;
}

TEST(GpioTest, MoveOtherPinToInputPin)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::output, true)).Times(1);
    std::unique_ptr<hal::OutputPin> movedFromPin{ std::make_unique<hal::OutputPin>(pin, true) };

    EXPECT_CALL(pin, ReConfig(hal::PinConfigType::input)).Times(1);
    std::unique_ptr<hal::InputPin> movedToPin{ std::make_unique<hal::InputPin>(std::move(*movedFromPin)) };

    movedFromPin = nullptr;

    EXPECT_CALL(pin, ResetConfig()).Times(1);
    movedToPin = nullptr;
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

TEST(GpioTest, MoveOutputPinToOutputPinWithInitial)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::output, false)).Times(1);
    std::unique_ptr<hal::OutputPin> movedFromPin{ std::make_unique<hal::OutputPin>(pin, false) };

    EXPECT_CALL(pin, Set(true)).Times(1);
    std::unique_ptr<hal::OutputPin> movedToPin{ std::make_unique<hal::OutputPin>(std::move(*movedFromPin), true) };

    movedFromPin = nullptr;

    EXPECT_CALL(pin, ResetConfig()).Times(1);
    movedToPin = nullptr;
}

TEST(GpioTest, MoveOtherPinToOutputPin)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::input)).Times(1);
    std::unique_ptr<hal::InputPin> movedFromPin{ std::make_unique<hal::InputPin>(pin) };

    EXPECT_CALL(pin, ReConfig(hal::PinConfigType::output, true)).Times(1);
    std::unique_ptr<hal::OutputPin> movedToPin{ std::make_unique<hal::OutputPin>(std::move(*movedFromPin), true) };

    movedFromPin = nullptr;

    EXPECT_CALL(pin, ResetConfig()).Times(1);
    movedToPin = nullptr;
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
    inputPin.EnableInterrupt(infra::emptyFunction, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched);

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

TEST(GpioTest, MoveTriStatePinToTriState_as_input)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::triState)).Times(1);
    std::unique_ptr<hal::TriStatePin> movedFromPin{ std::make_unique<hal::TriStatePin>(pin) };

    EXPECT_CALL(pin, SetAsInput()).Times(1);
    std::unique_ptr<hal::TriStatePin> movedToPin{ std::make_unique<hal::TriStatePin>(std::move(*movedFromPin)) };

    movedFromPin = nullptr;

    EXPECT_CALL(pin, ResetConfig()).Times(1);
    movedToPin = nullptr;
}

TEST(GpioTest, MoveTriStatePinToTriState_as_output)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::triState, false)).Times(1);
    std::unique_ptr<hal::TriStatePin> movedFromPin{ std::make_unique<hal::TriStatePin>(pin, false) };

    EXPECT_CALL(pin, Set(true)).Times(1);
    std::unique_ptr<hal::TriStatePin> movedToPin{ std::make_unique<hal::TriStatePin>(std::move(*movedFromPin), true) };

    movedFromPin = nullptr;

    EXPECT_CALL(pin, ResetConfig()).Times(1);
    movedToPin = nullptr;
}

TEST(GpioTest, MoveOtherPinToTriStatePin_as_input)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::input)).Times(1);
    std::unique_ptr<hal::InputPin> movedFromPin{ std::make_unique<hal::InputPin>(pin) };

    EXPECT_CALL(pin, ReConfig(hal::PinConfigType::triState)).Times(1);
    std::unique_ptr<hal::TriStatePin> movedToPin{ std::make_unique<hal::TriStatePin>(std::move(*movedFromPin)) };

    movedFromPin = nullptr;

    EXPECT_CALL(pin, ResetConfig()).Times(1);
    movedToPin = nullptr;
}

TEST(GpioTest, MoveOtherPinToTriStatePin_as_output)
{
    testing::StrictMock<hal::GpioPinMock> pin;

    EXPECT_CALL(pin, Config(hal::PinConfigType::input)).Times(1);
    std::unique_ptr<hal::InputPin> movedFromPin{ std::make_unique<hal::InputPin>(pin) };

    EXPECT_CALL(pin, ReConfig(hal::PinConfigType::triState, true)).Times(1);
    std::unique_ptr<hal::TriStatePin> movedToPin{ std::make_unique<hal::TriStatePin>(std::move(*movedFromPin), true) };

    movedFromPin = nullptr;

    EXPECT_CALL(pin, ResetConfig()).Times(1);
    movedToPin = nullptr;
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

    pin.EnableInterrupt(infra::emptyFunction, hal::InterruptTrigger::bothEdges, hal::InterruptType::dispatched);
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
