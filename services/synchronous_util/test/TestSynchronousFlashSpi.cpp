#include "gtest/gtest.h"
#include "hal/synchronous_interfaces/test_doubles/SynchronousSpiMock.hpp"
#include "services/synchronous_util/SynchronousFlashSpi.hpp"

class SynchronousFlashSpiTest
    : public testing::Test
{
protected:
    SynchronousFlashSpiTest(bool extendedAddressing)
        : config{ 512, extendedAddressing }
        , flashSpi(spiMock, config)
    {}

    void ExpectWriteEnable()
    {
        EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x06 }, hal::SynchronousSpi::Action::stop));
    }

    void ExpectHoldWhileWriteInProgress()
    {
        EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x05 }, hal::SynchronousSpi::Action::continueSession));
        EXPECT_CALL(spiMock, ReceiveDataMock(hal::SynchronousSpi::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    }

    services::SynchronousFlashSpi::Config config;
    testing::StrictMock<hal::SynchronousSpiMock> spiMock;
    services::SynchronousFlashSpi flashSpi;

    testing::InSequence s;
};

class SynchronousFlashSpiTest3ByteAddressing : public SynchronousFlashSpiTest
{
public:
    SynchronousFlashSpiTest3ByteAddressing()
        : SynchronousFlashSpiTest(false)
    {}
};

TEST_F(SynchronousFlashSpiTest3ByteAddressing, WriteBuffer)
{
    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x02, 0x12, 0x34, 0x56 }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    std::array<uint8_t, 8> data = { 0, 1, 2, 3, 4, 5, 6, 7 };
    flashSpi.WriteBuffer(data, 0x123456);
}

TEST_F(SynchronousFlashSpiTest3ByteAddressing, ReadBuffer)
{
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x03, 0x12, 0x34, 0x56 }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SynchronousSpi::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }));

    std::array<uint8_t, 8> buffer;
    flashSpi.ReadBuffer(buffer, 0x123456);

    EXPECT_THAT(buffer, testing::ElementsAre(0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07));
}

TEST_F(SynchronousFlashSpiTest3ByteAddressing, EraseSectors)
{
    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x20, 0x00, 0x00, 0x00 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x20, 0x00, 0x10, 0x00 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    flashSpi.EraseSectors(0, 2);
}

TEST_F(SynchronousFlashSpiTest3ByteAddressing, ReadFlashId)
{
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x9f }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SynchronousSpi::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0x01, 0x02, 0x03 }));

    std::array<uint8_t, 3> flashId;
    flashSpi.ReadFlashId(flashId);

    EXPECT_THAT(flashId, testing::ElementsAre(0x01, 0x02, 0x03));
}

class SynchronousFlashSpiTest4ByteAddressing : public SynchronousFlashSpiTest
{
public:
    SynchronousFlashSpiTest4ByteAddressing()
        : SynchronousFlashSpiTest(true)
    {}
};

TEST_F(SynchronousFlashSpiTest4ByteAddressing, WriteBuffer)
{
    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x12, 0x12, 0x34, 0x56, 0x78 }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0, 1, 2, 3, 4, 5, 6, 7 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    std::array<uint8_t, 8> data = { 0, 1, 2, 3, 4, 5, 6, 7 };
    flashSpi.WriteBuffer(data, 0x12345678);
}

TEST_F(SynchronousFlashSpiTest4ByteAddressing, ReadBuffer)
{
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x13, 0x12, 0x34, 0x56, 0x78 }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SynchronousSpi::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }));

    std::array<uint8_t, 8> buffer;
    flashSpi.ReadBuffer(buffer, 0x12345678);

    EXPECT_THAT(buffer, testing::ElementsAre(0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07));
}

TEST_F(SynchronousFlashSpiTest4ByteAddressing, EraseSectors)
{
    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x21, 0x00, 0x00, 0x00, 0x00 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    ExpectWriteEnable();
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x21, 0x00, 0x00, 0x10, 0x00 }, hal::SynchronousSpi::Action::stop));
    ExpectHoldWhileWriteInProgress();

    flashSpi.EraseSectors(0, 2);
}

TEST_F(SynchronousFlashSpiTest4ByteAddressing, ReadFlashId)
{
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 0x9f }, hal::SynchronousSpi::Action::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SynchronousSpi::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0x01, 0x02, 0x03 }));

    std::array<uint8_t, 3> flashId;
    flashSpi.ReadFlashId(flashId);

    EXPECT_THAT(flashId, testing::ElementsAre(0x01, 0x02, 0x03));
}

