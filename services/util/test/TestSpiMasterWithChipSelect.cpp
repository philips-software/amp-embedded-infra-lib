#include "hal/interfaces/test_doubles/CommunicationConfiguratorMock.hpp"
#include "hal/interfaces/test_doubles/GpioStub.hpp"
#include "hal/interfaces/test_doubles/SpiMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "services/util/SpiMasterWithChipSelect.hpp"
#include "gtest/gtest.h"

class SpiMasterWithChipSelectTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    SpiMasterWithChipSelectTest()
    {
        EXPECT_CALL(spiMock, SetChipSelectConfiguratorMock(testing::_));
        spi.emplace(spiMock, chipSelect);
    }

    testing::StrictMock<hal::SpiAsynchronousMock> spiMock;
    hal::GpioPinStub chipSelect;
    infra::Optional<services::SpiMasterWithChipSelect> spi;
};

TEST_F(SpiMasterWithChipSelectTest, ChipSelectStartsHigh)
{
    EXPECT_TRUE(chipSelect.GetStubState());
}

TEST_F(SpiMasterWithChipSelectTest, on_StartSession_chip_select_is_activated)
{
    std::vector<uint8_t> buffer = { 0 };
    EXPECT_CALL(spiMock, SendAndReceiveMock(buffer, infra::MemoryRange<uint8_t>(buffer), hal::SpiAction::stop, testing::_));

    spi->StartSession();
    spi->SendAndReceive(buffer, buffer, hal::SpiAction::stop, infra::emptyFunction);

    EXPECT_FALSE(chipSelect.GetStubState());
}

TEST_F(SpiMasterWithChipSelectTest, on_EndSession_chip_select_is_deactivated)
{
    std::vector<uint8_t> buffer = { 0 };
    EXPECT_CALL(spiMock, SendAndReceiveMock(buffer, infra::MemoryRange<uint8_t>(buffer), hal::SpiAction::stop, testing::_));

    spi->StartSession();
    spi->SendAndReceive(buffer, buffer, hal::SpiAction::stop, infra::emptyFunction);
    spi->EndSession();

    EXPECT_TRUE(chipSelect.GetStubState());
}

TEST_F(SpiMasterWithChipSelectTest, additional_ChipSelectConfigurator_is_invoked)
{
    std::vector<uint8_t> buffer = { 0 };
    hal::ChipSelectConfiguratorMock chipSelectConfiguratorMock;
    spi->SetChipSelectConfigurator(chipSelectConfiguratorMock);

    EXPECT_CALL(spiMock, SendAndReceiveMock(buffer, infra::MemoryRange<uint8_t>(buffer), hal::SpiAction::stop, testing::_));

    EXPECT_CALL(chipSelectConfiguratorMock, StartSession());
    spi->StartSession();
    spi->SendAndReceive(buffer, buffer, hal::SpiAction::stop, infra::emptyFunction);
    EXPECT_CALL(chipSelectConfiguratorMock, EndSession());
    spi->EndSession();
}

TEST_F(SpiMasterWithChipSelectTest, SetCommunicationConfigurator_is_forwarded)
{
    hal::CommunicationConfiguratorMock configurator;

    EXPECT_CALL(spiMock, SetCommunicationConfigurator(testing::Ref(configurator)));
    spi->SetCommunicationConfigurator(configurator);

    EXPECT_CALL(spiMock, ResetCommunicationConfigurator());
    spi->ResetCommunicationConfigurator();
}
