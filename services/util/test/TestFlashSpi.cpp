#include "hal/interfaces/test_doubles/SpiMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/FlashSpi.hpp"
#include "gmock/gmock.h"

class FlashSpiTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    FlashSpiTest()
        : flash(spiMock)
    {}

    std::vector<uint8_t> CreateInstruction(uint8_t instruction)
    {
        return std::vector<uint8_t>{ instruction };
    }

    std::vector<uint8_t> CreateInstructionAndAddress(uint8_t instruction, uint32_t address)
    {
        return std::vector<uint8_t>{ instruction, static_cast<uint8_t>(address >> 16), static_cast<uint8_t>(address >> 8), static_cast<uint8_t>(address) };
    }

    std::vector<uint8_t> CreateInstructionAddressAndData(uint8_t instruction, uint32_t address, const std::vector<uint8_t>& data)
    {
        std::vector<uint8_t> result = CreateInstructionAndAddress(instruction, address);
        result.insert(result.end(), data.begin(), data.end());
        return result;
    }

    testing::StrictMock<hal::SpiMock> spiMock;
    services::FlashSpi flash;

    testing::StrictMock<infra::MockCallback<void()>> finished;
};

TEST_F(FlashSpiTest, Construction)
{
    EXPECT_EQ(512, flash.NumberOfSectors());
    EXPECT_EQ(4096, flash.SizeOfSector(0));
}

TEST_F(FlashSpiTest, ReadData)
{
    std::vector<uint8_t> receiveData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandReadData, 0), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop))
        .WillOnce(testing::Return(receiveData));
    EXPECT_CALL(finished, callback());

    std::vector<uint8_t> buffer(4, 0);
    flash.ReadBuffer(buffer, 0, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();

    EXPECT_EQ(receiveData, buffer);
}

TEST_F(FlashSpiTest, ReadId)
{
    std::vector<uint8_t> receiveData = { 1, 2, 3 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadId), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(receiveData));
    EXPECT_CALL(finished, callback());

    std::vector<uint8_t> buffer(3, 0);
    flash.ReadFlashId(buffer, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();

    EXPECT_EQ(receiveData, buffer);
}

TEST_F(FlashSpiTest, ReadDataAtNonZeroAddress)
{
    std::vector<uint8_t> receiveData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandReadData, 0x123456), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop))
        .WillOnce(testing::Return(receiveData));
    EXPECT_CALL(finished, callback());

    std::vector<uint8_t> buffer(4, 0);
    flash.ReadBuffer(buffer, 0 + 0x123456, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();

    EXPECT_EQ(receiveData, buffer);
}

TEST_F(FlashSpiTest, WriteData)
{
    const std::vector<uint8_t> sendData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandPageProgram, 0), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(sendData, hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.WriteBuffer(sendData, 0, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, WriteDataFinishesOnFlagPoll)
{
    const std::vector<uint8_t> sendData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandPageProgram, 0), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(sendData, hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    EXPECT_CALL(finished, callback());

    flash.WriteBuffer(sendData, 0, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, WriteDataAtNonZeroAddress)
{
    const std::vector<uint8_t> sendData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandPageProgram, 0x123456), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(sendData, hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.WriteBuffer(sendData, 0 + 0x123456, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, WhenPageBoundaryIsCrossedFirstPartIsWritten)
{
    const std::vector<uint8_t> sendData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandPageProgram, 254), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 1, 2 }, hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.WriteBuffer(sendData, 0 + 254, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, WhenPageBoundaryIsCrossedSecondPartIsWritten)
{
    testing::InSequence s;

    const std::vector<uint8_t> sendData = { 1, 2, 3, 4, 5, 6 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandPageProgram, 254), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 1, 2 }, hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandPageProgram, 256), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(std::vector<uint8_t>{ 3, 4, 5, 6 }, hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.WriteBuffer(sendData, 0 + 254, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, EraseFirstSubSector)
{
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, 0), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.EraseSector(0, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, EraseFirstSubSectorFinishesOnFlagPoll)
{
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, 0), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    EXPECT_CALL(finished, callback());

    flash.EraseSector(0, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, EraseSecondSubSector)
{
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, 4096), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.EraseSector(1, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, EraseMultipleErasesOneSubSector)
{
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, 0), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.EraseSectors(0, 1, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, EraseMultipleErasesTwoSubSectors)
{
    testing::InSequence s;

    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, 0), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, 4096), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.EraseSectors(0, 2, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, EraseMultipleErasesSector)
{
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSector, 0), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.EraseSectors(0, 16, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, EraseMultipleErasesSubSectorsAndSector)
{
    testing::InSequence s;

    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, 61440), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSector, 65536), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, 131072), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.EraseSectors(15, 33, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_F(FlashSpiTest, EraseAllErasesBulk)
{
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandEraseBulk), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.EraseAll([this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}
