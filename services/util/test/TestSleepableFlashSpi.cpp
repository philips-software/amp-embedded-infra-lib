#include "hal/interfaces/test_doubles/SpiMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/SleepableFlashSpi.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace services
{
    class SleepableFlashSpiTest
        : public testing::Test
        , public infra::ClockFixture
    {
    public:
        std::vector<uint8_t> CreateInstruction(uint8_t instruction)
        {
            return std::vector<uint8_t>{ instruction };
        }

        FlashSpi::Config FlashSpiConfig()
        {
            FlashSpi::Config config{};
            return config;
        }

        testing::StrictMock<hal::SpiMock> spiMock;
        SleepableFlashSpi flash{ spiMock, FlashSpiConfig() };

        testing::StrictMock<infra::MockCallback<void()>> finished;
    };

    TEST_F(SleepableFlashSpiTest, SleepCommand)
    {
        EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::SleepableFlashSpi::commandDeepPowerDown), hal::SpiAction::stop));
        infra::VerifyingFunction<void()> onDone;
        this->flash.Sleep(onDone);

        ExecuteAllActions();
    }

    TEST_F(SleepableFlashSpiTest, WakeCommand)
    {
        EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::SleepableFlashSpi::commandReleaseFromDeepPowerDown), hal::SpiAction::stop));
        infra::VerifyingFunction<void()> onDone;
        this->flash.Wake(onDone);

        ExecuteAllActions();
    }

    TEST_F(SleepableFlashSpiTest, WakeBeforeSleepFinished)
    {
        EXPECT_CALL(spiMock, SendDataMock(testing::_, testing::_)).Times(testing::AnyNumber());
        infra::VerifyingFunction<void()> onSleepDone;
        infra::VerifyingFunction<void()> onWakeDone;
        this->flash.Sleep(onSleepDone);
        this->flash.Wake(onWakeDone);

        ExecuteAllActions();
    }
}
