#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/GpioStub.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "services/util/DebugLed.hpp"

class DebugLedTestBase
    : public testing::Test
    , public infra::ClockFixture
{
public:
    infra::Optional<services::DebugLed> debugLed;
    hal::GpioPinSpy led;
};

class DebugLedTest
    : public DebugLedTestBase
{
public:
    DebugLedTest()
    {
        debugLed.Emplace(led);
    }
};

TEST_F(DebugLedTest, DebugLedContinuouslyToggles)
{
    ForwardTime(std::chrono::seconds(2));
    EXPECT_EQ((std::vector<hal::PinChange>{
                  { std::chrono::milliseconds(800), true },
                  { std::chrono::milliseconds(1000), false },
                  { std::chrono::milliseconds(1800), true },
                  { std::chrono::milliseconds(2000), false } }),
        led.PinChanges());
}

TEST_F(DebugLedTestBase, NonStandardDurations)
{
    debugLed.Emplace(led, std::chrono::milliseconds(20), std::chrono::milliseconds(80));

    ForwardTime(std::chrono::milliseconds(200));
    EXPECT_EQ((std::vector<hal::PinChange>{
                  { std::chrono::milliseconds(80), true },
                  { std::chrono::milliseconds(100), false },
                  { std::chrono::milliseconds(180), true },
                  { std::chrono::milliseconds(200), false } }),
        led.PinChanges());
}
