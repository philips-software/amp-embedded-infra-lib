#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/GpioStub.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/DebouncedButton.hpp"

class DebouncedButtonFixtureBase
{
public:
};

class DebouncedButtonFixture
    : public testing::Test
    , public infra::ClockFixture
{
public:
    DebouncedButtonFixture()
        : debouncedButton(button, [this]() { onPressed.callback(); }, [this]() { onReleased.callback(); })
    {}

    hal::GpioPinStub button;
    infra::MockCallback<void()> onPressed;
    infra::MockCallback<void()> onReleased;
    services::DebouncedButton debouncedButton;
};

TEST_F(DebouncedButtonFixture, onPressed_triggered_when_pushed)
{
    EXPECT_CALL(onPressed, callback());

    button.SetStubState(true);
}

TEST_F(DebouncedButtonFixture, onReleased_triggered_when_released_outside_the_debounce_period)
{
    EXPECT_CALL(onPressed, callback());
    button.SetStubState(true);

    ForwardTime(std::chrono::seconds(1));

    EXPECT_CALL(onReleased, callback());
    button.SetStubState(false);
}

TEST_F(DebouncedButtonFixture, onReleased_triggered_after_debounce_period)
{
    EXPECT_CALL(onPressed, callback());
    button.SetStubState(true);

    ForwardTime(std::chrono::milliseconds(5));
    button.SetStubState(false);

    EXPECT_CALL(onReleased, callback());
    ForwardTime(std::chrono::milliseconds(5));
}

TEST_F(DebouncedButtonFixture, onRelease_not_triggered_when_pushed_in_the_debounce_period)
{
    EXPECT_CALL(onPressed, callback()).With(After(std::chrono::milliseconds(0)));

    button.SetStubState(true);
    button.SetStubState(false);
    ForwardTime(std::chrono::milliseconds(5));
    button.SetStubState(true);
}

TEST_F(DebouncedButtonFixture, triggered_again_when_pushed_outside_the_debounce_period)
{
    EXPECT_CALL(onPressed, callback()).With(After(std::chrono::milliseconds(0)));
    EXPECT_CALL(onPressed, callback()).With(After(std::chrono::milliseconds(10)));

    button.SetStubState(true);
    button.SetStubState(false);
    ForwardTime(std::chrono::milliseconds(10));
    button.SetStubState(true);
}

TEST_F(DebouncedButtonFixture, onPressed_not_triggered_when_pushed_in_the_onRelease_debounce_period)
{
    EXPECT_CALL(onPressed, callback()).With(After(std::chrono::milliseconds(0)));

    button.SetStubState(true);
    ForwardTime(std::chrono::milliseconds(10));

    EXPECT_CALL(onReleased, callback()).With(After(std::chrono::milliseconds(0)));
    button.SetStubState(false);
    ForwardTime(std::chrono::milliseconds(5));
    button.SetStubState(true);
}

TEST_F(DebouncedButtonFixture, onPressed_triggered_after_debounce_period)
{
    EXPECT_CALL(onPressed, callback());

    button.SetStubState(true);
    ForwardTime(std::chrono::milliseconds(10));

    EXPECT_CALL(onReleased, callback());
    button.SetStubState(false);
    ForwardTime(std::chrono::milliseconds(5));
    button.SetStubState(true);

    EXPECT_CALL(onPressed, callback());
    ForwardTime(std::chrono::milliseconds(5));
}
