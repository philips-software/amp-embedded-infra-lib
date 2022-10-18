#include "hal/interfaces/test_doubles/CommunicationConfiguratorMock.hpp"
#include "hal/interfaces/test_doubles/GpioStub.hpp"
#include "hal/interfaces/test_doubles/SpiMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/LowPowerSpiMaster.hpp"
#include "gtest/gtest.h"

class LowPowerSpiMasterTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    LowPowerSpiMasterTest()
        : lowPowerSpiMaster(spiMock, mainClock)
    {}

    testing::StrictMock<hal::SpiMock> spiMock;
    infra::MainClockReference mainClock;
    hal::LowPowerSpiMaster lowPowerSpiMaster;
};

TEST_F(LowPowerSpiMasterTest, construction)
{
    EXPECT_FALSE(mainClock.IsReferenced());
}

TEST_F(LowPowerSpiMasterTest, SetChipSelectConfigurator_will_forward_call_downstream)
{
    hal::ChipSelectConfiguratorMock configurator;

    EXPECT_CALL(spiMock, SetChipSelectConfigurator(testing::Ref(configurator)));

    lowPowerSpiMaster.SetChipSelectConfigurator(configurator);
}

TEST_F(LowPowerSpiMasterTest, SetCommunicationConfigurator_will_forward_call_downstream)
{
    hal::CommunicationConfiguratorMock configurator;

    EXPECT_CALL(spiMock, SetCommunicationConfigurator(testing::Ref(configurator)));

    lowPowerSpiMaster.SetCommunicationConfigurator(configurator);
}

TEST_F(LowPowerSpiMasterTest, ResetCommunicationConfigurator_will_forward_call_downstream)
{
    EXPECT_CALL(spiMock, ResetCommunicationConfigurator());

    lowPowerSpiMaster.ResetCommunicationConfigurator();
}

TEST_F(LowPowerSpiMasterTest, SendAndReceive_will_refere_mainClock_and_release_onDone)
{
    infra::MockCallback<void()> mockOnDone;
    std::vector<uint8_t> buffer = { 0 };

    EXPECT_CALL(spiMock, SendDataMock(testing::_, testing::_));
    EXPECT_CALL(spiMock, ReceiveDataMock(testing::_)).WillOnce(testing::Return(buffer));

    lowPowerSpiMaster.SendAndReceive(buffer, buffer, hal::SpiAction::stop, [&mockOnDone]()
        { mockOnDone.callback(); });

    EXPECT_TRUE(mainClock.IsReferenced());

    EXPECT_CALL(mockOnDone, callback());
    ExecuteAllActions();

    EXPECT_FALSE(mainClock.IsReferenced());
}