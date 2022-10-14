#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/GpioStub.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/RepeatingButton.hpp"

class RepeatingButtonFixtureBase
    : public testing::Test
    , public infra::ClockFixture
{
public:
    infra::Optional<services::RepeatingButton> repeatingButton;
    services::RepeatingButton::Config config;
    hal::GpioPinStub button;
    infra::MockCallback<void()> callback;
};

class RepeatingButtonFixture
    : public RepeatingButtonFixtureBase
{
public:
    RepeatingButtonFixture()
    {
        repeatingButton.Emplace(
            button, [this]()
            { callback.callback(); },
            config);
    }
};

TEST_F(RepeatingButtonFixture, TriggeredWhenPushed)
{
    EXPECT_CALL(callback, callback());

    button.SetStubState(true);
}

TEST_F(RepeatingButtonFixture, NotTriggeredWhenPushedInTheDebouncePeriod)
{
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(0)));

    button.SetStubState(true);
    button.SetStubState(false);
    ForwardTime(std::chrono::milliseconds(5));
    button.SetStubState(true);
}

TEST_F(RepeatingButtonFixture, TriggeredAgainWhenPushedOutsideTheDebouncePeriod)
{
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(0)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(10)));

    button.SetStubState(true);
    button.SetStubState(false);
    ForwardTime(std::chrono::milliseconds(10));
    button.SetStubState(true);
}

TEST_F(RepeatingButtonFixture, RepeatedTriggerStartsAfter500ms)
{
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(0)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(500)));

    button.SetStubState(true);
    ForwardTime(std::chrono::milliseconds(500));
}

TEST_F(RepeatingButtonFixture, RepeatedTriggerStartsAfter500msEvenWithBounce)
{
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(0)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(500)));

    button.SetStubState(true);
    button.SetStubState(false);
    ForwardTime(std::chrono::milliseconds(5));
    button.SetStubState(true);
    ForwardTime(std::chrono::milliseconds(495));
}

TEST_F(RepeatingButtonFixture, RepeatingButtonContinuouslyFires)
{
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(0)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(500)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(750)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(1000)));

    button.SetStubState(true);
    ForwardTime(std::chrono::seconds(1));
}

TEST_F(RepeatingButtonFixtureBase, NonDefaultTimings)
{
    config.debounceDuration = std::chrono::milliseconds(20);
    config.initialDelay = std::chrono::seconds(2);
    config.successiveDelay = std::chrono::seconds(1);
    repeatingButton.Emplace(
        button, [this]()
        { callback.callback(); },
        config);

    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(0)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(2000)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(3000)));
    EXPECT_CALL(callback, callback()).With(After(std::chrono::milliseconds(4000)));

    button.SetStubState(true);
    button.SetStubState(false);
    ForwardTime(std::chrono::milliseconds(15));
    button.SetStubState(true);
    ForwardTime(std::chrono::milliseconds(3985));
}
