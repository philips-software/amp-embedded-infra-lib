#include "hal/interfaces/Gpio.hpp"
#include "hal/interfaces/test_doubles/GpioMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "services/util/InputPinMultipleAccess.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class InputPinMultipleAccessTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    testing::StrictMock<hal::GpioPinMock> gpioPinMock;
};

class ParameterizedInputPinMultipleAccessTest
    : public InputPinMultipleAccessTest
    , public testing::WithParamInterface<hal::InterruptType>
{};

INSTANTIATE_TEST_SUITE_P(InputPinMultipleAccessTest, ParameterizedInputPinMultipleAccessTest, ::testing::Values(hal::InterruptType::dispatched, hal::InterruptType::immediate));

TEST_F(InputPinMultipleAccessTest, master_enables_interrupt_by_default_disables_on_destruction)
{
    EXPECT_CALL(gpioPinMock, Config(hal::PinConfigType::input));

    EXPECT_CALL(gpioPinMock, EnableInterrupt(testing::_, hal::InterruptTrigger::bothEdges, hal::InterruptType::immediate));
    services::InputPinMultipleAccessMaster inputMaster{ gpioPinMock };

    EXPECT_CALL(gpioPinMock, DisableInterrupt());
    EXPECT_CALL(gpioPinMock, ResetConfig());
}

TEST_P(ParameterizedInputPinMultipleAccessTest, action_called_on_both_edges)
{
    hal::InterruptType type = GetParam();

    EXPECT_CALL(gpioPinMock, Config(hal::PinConfigType::input));

    infra::Function<void()> interruptCallback = infra::emptyFunction;
    EXPECT_CALL(gpioPinMock, EnableInterrupt(testing::_, hal::InterruptTrigger::bothEdges, hal::InterruptType::immediate))
        .WillOnce(testing::SaveArg<0>(&interruptCallback));
    services::InputPinMultipleAccessMaster inputMaster{ gpioPinMock };

    bool risingCalled = false;
    services::InputPinMultipleAccess rising{ inputMaster };
    rising.EnableInterrupt([&risingCalled]
        {
            risingCalled = true;
        },
        hal::InterruptTrigger::risingEdge, type);

    bool fallingCalled = false;
    services::InputPinMultipleAccess falling{ inputMaster };
    falling.EnableInterrupt([&fallingCalled]
        {
            fallingCalled = true;
        },
        hal::InterruptTrigger::fallingEdge, type);

    // Rising edge
    EXPECT_CALL(gpioPinMock, Get()).WillRepeatedly(testing::Return(true));
    interruptCallback();
    if (type == hal::InterruptType::dispatched)
    {
        ExecuteAllActions();
    }
    EXPECT_THAT(risingCalled, testing::IsTrue());
    EXPECT_THAT(fallingCalled, testing::IsFalse());

    risingCalled = false;
    fallingCalled = false;

    // Falling edge
    EXPECT_CALL(gpioPinMock, Get()).WillRepeatedly(testing::Return(false));
    interruptCallback();
    if (type == hal::InterruptType::dispatched)
    {
        ExecuteAllActions();
    }
    EXPECT_THAT(risingCalled, testing::IsFalse());
    EXPECT_THAT(fallingCalled, testing::IsTrue());

    EXPECT_CALL(gpioPinMock, DisableInterrupt());
    EXPECT_CALL(gpioPinMock, ResetConfig());
}
