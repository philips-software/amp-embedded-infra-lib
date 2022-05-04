#include "gtest/gtest.h"
#include "hal/synchronous_interfaces/test_doubles/SynchronousSpiMock.hpp"
#include "services/synchronous_util/SynchronousFlashSpi.hpp"

class SynchronousFlashSpiTest
    : public testing::Test
{
public:
    SynchronousFlashSpiTest()
        : flashSpi(spiMock)
    {
    }

    void ExpectWriteEnable()
    {
        EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x06 }, hal::SynchronousSpi::Action::stop));
    }

    void ExpectHoldWhileWriteInProgress()
    {
        EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x05 }, hal::SynchronousSpi::Action::continueSession));
        EXPECT_CALL(spiMock, ReceiveDataMock(hal::SynchronousSpi::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    }

    testing::StrictMock<hal::SynchronousSpiMock> spiMock;
    services::SynchronousFlashSpi flashSpi;

    testing::InSequence s;
};

TEST_F(SynchronousFlashSpiTest, WriteBuffer)
{
    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x02, 0x12, 0x34, 0x56 }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    std::array<uint8_t, 8> data = { 0, 1, 2, 3, 4, 5, 6, 7 };
    flashSpi.WriteBuffer(data, 0x123456);
}

TEST_F(SynchronousFlashSpiTest, ReadBuffer)
{
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x03, 0x12, 0x34, 0x56 }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SynchronousSpi::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }));

    std::array<uint8_t, 8> buffer;
    flashSpi.ReadBuffer(buffer, 0x123456);

    ASSERT_THAT(buffer, testing::ElementsAre(0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07));
}

TEST_F(SynchronousFlashSpiTest, EraseSectors)
{
    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x20, 0x00, 0x00, 0x00 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x20, 0x00, 0x10, 0x00 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    flashSpi.EraseSectors(0, 2);
}

TEST_F(SynchronousFlashSpiTest, ReadFlashId)
{
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x9f }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SynchronousSpi::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0x01, 0x02, 0x03 }));

    std::array<uint8_t, 3> flashId;
    flashSpi.ReadFlashId(flashId);

    ASSERT_THAT(flashId, testing::ElementsAre(0x01, 0x02, 0x03));
}
