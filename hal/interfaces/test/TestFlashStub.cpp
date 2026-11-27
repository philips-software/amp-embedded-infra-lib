#include "hal/interfaces/test_doubles/FlashStub.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class FlashStubTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    hal::FlashStub flash{ 2, 4 };
};

TEST_F(FlashStubTest, delayed_erase_schedules_nothing)
{
    infra::MockCallback<void()> onDone;
    flash.delaySignalEraseDone = true;
    flash.sectors[0] = { 1, 2, 3, 4 };

    flash.EraseSectors(0, 1, [&onDone]
        {
            onDone.callback();
        });

    EXPECT_TRUE(infra::EventDispatcher::Instance().IsIdle());
    EXPECT_EQ((std::vector<uint8_t>{ 0xff, 0xff, 0xff, 0xff }), flash.sectors[0]);
}

TEST_F(FlashStubTest, delayed_erase_signals_done_through_onEraseDone)
{
    infra::MockCallback<void()> onDone;
    flash.delaySignalEraseDone = true;

    flash.EraseSectors(0, 1, [&onDone]
        {
            onDone.callback();
        });
    ExecuteAllActions();

    EXPECT_CALL(onDone, callback());
    flash.onEraseDone();
}

TEST_F(FlashStubTest, erase_without_delay_signals_done)
{
    infra::MockCallback<void()> onDone;

    flash.EraseSectors(0, 1, [&onDone]
        {
            onDone.callback();
        });

    EXPECT_CALL(onDone, callback());
    ExecuteAllActions();
}
