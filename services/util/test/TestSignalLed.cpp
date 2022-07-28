#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/GpioStub.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/SignalLed.hpp"

class SignalLedTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    SignalLedTest()
        : signalLed(led, 0, 0, std::chrono::seconds(1))
    {}

    hal::GpioPinSpy led;
    services::SignalLed signalLed;
};

TEST_F(SignalLedTest, StayOffWhenNoPatternIsLoaded)
{
    EXPECT_FALSE(led.GetOutputLatch());
}

TEST_F(SignalLedTest, TurnOnImmediatelyWhenPatternIsOn)
{
    signalLed.Set(1, 2);

    EXPECT_TRUE(led.GetOutputLatch());
}

TEST_F(SignalLedTest, StayOffWhenPatternIsOff)
{
    signalLed.Set(0, 2);

    EXPECT_FALSE(led.GetOutputLatch());
}

TEST_F(SignalLedTest, TurnOnAfterPeriod)
{
    signalLed.Set(2, 2);

    ForwardTime(std::chrono::seconds(1));
    EXPECT_EQ((std::vector<hal::PinChange>{ { std::chrono::seconds(1), true } }), led.PinChanges());
}

TEST_F(SignalLedTest, TurnOffAfterPeriod)
{
    signalLed.Set(1, 2);

    ForwardTime(std::chrono::seconds(1));
    EXPECT_EQ((std::vector<hal::PinChange>{ { std::chrono::seconds(0), true }, { std::chrono::seconds(1), false } }), led.PinChanges());
}

TEST_F(SignalLedTest, TurnOnAndOffPeriodically)
{
    signalLed.Set(0xb, 4);  // binary: 1011

    ForwardTime(std::chrono::seconds(8));
    EXPECT_EQ((std::vector<hal::PinChange>{
        { std::chrono::seconds(0), true },
        { std::chrono::seconds(2), false },
        { std::chrono::seconds(3), true },
        { std::chrono::seconds(6), false },
        { std::chrono::seconds(7), true }
        }), led.PinChanges());
}
