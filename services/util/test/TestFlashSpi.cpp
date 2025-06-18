#include "hal/interfaces/test_doubles/SpiMock.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/FlashSpi.hpp"
#include "gmock/gmock.h"

struct FlashSpiTestArguments
{
    bool extendedAddressing;
    uint32_t address;
    uint32_t nrOfSubSectors;
};

class FlashSpiTest
    : public testing::Test
    , public infra::ClockFixture
    , public testing::WithParamInterface<FlashSpiTestArguments>
{
public:
    std::vector<uint8_t> CreateInstruction(uint8_t instruction)
    {
        return std::vector<uint8_t>{ instruction };
    }

    std::vector<uint8_t> CreateInstructionAndAddress(const std::array<uint8_t, 2>& instruction, uint32_t address)
    {
        if (GetParam().extendedAddressing)
            return std::vector<uint8_t>{ instruction[1], static_cast<uint8_t>(address >> 24), static_cast<uint8_t>(address >> 16), static_cast<uint8_t>(address >> 8), static_cast<uint8_t>(address) };
        else
            return std::vector<uint8_t>{ instruction[0], static_cast<uint8_t>(address >> 16), static_cast<uint8_t>(address >> 8), static_cast<uint8_t>(address) };
    }

    std::vector<uint8_t> CreateInstructionAddressAndData(const std::array<uint8_t, 2>& instruction, uint32_t address, const std::vector<uint8_t>& data)
    {
        std::vector<uint8_t> result = CreateInstructionAndAddress(instruction, address);
        result.insert(result.end(), data.begin(), data.end());
        return result;
    }

    services::FlashSpi::Config FlashSpiConfig()
    {
        services::FlashSpi::Config config{};
        config.extendedAddressing = GetParam().extendedAddressing;
        config.nrOfSubSectors = GetParam().nrOfSubSectors;
        return config;
    }

    testing::StrictMock<hal::SpiMock> spiMock;
    services::FlashSpi flash{ spiMock, FlashSpiConfig() };

    testing::StrictMock<infra::MockCallback<void()>> finished;
};

TEST_P(FlashSpiTest, Construction)
{
    EXPECT_EQ(GetParam().nrOfSubSectors, flash.NumberOfSectors());
    EXPECT_EQ(4096, flash.SizeOfSector(0));
}

TEST_P(FlashSpiTest, ConstructionWithConfig)
{
    services::FlashSpi::Config config;
    config.nrOfSubSectors = 1024;
    config.sizeSector = 8192;
    config.sizeSubSector = 2048;
    config.sizePage = 64;
    services::FlashSpi flashConfigured{ spiMock, config };

    EXPECT_EQ(1024, flashConfigured.NumberOfSectors());
    EXPECT_EQ(2048, flashConfigured.SizeOfSector(0));
}

TEST_P(FlashSpiTest, ReadData)
{
    std::vector<uint8_t> receiveData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandReadData, GetParam().address), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop))
        .WillOnce(testing::Return(receiveData));
    EXPECT_CALL(finished, callback());

    std::vector<uint8_t> buffer(4, 0);
    flash.ReadBuffer(buffer, GetParam().address, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();

    EXPECT_EQ(receiveData, buffer);
}

TEST_P(FlashSpiTest, ReadId)
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

TEST_P(FlashSpiTest, WriteData)
{
    const std::vector<uint8_t> sendData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandPageProgram, GetParam().address), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(sendData, hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.WriteBuffer(sendData, GetParam().address, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_P(FlashSpiTest, WriteDataFinishesOnFlagPoll)
{
    const std::vector<uint8_t> sendData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandPageProgram, GetParam().address), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, SendDataMock(sendData, hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 0 }));
    EXPECT_CALL(finished, callback());

    flash.WriteBuffer(sendData, GetParam().address, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_P(FlashSpiTest, WhenPageBoundaryIsCrossedFirstPartIsWritten)
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

TEST_P(FlashSpiTest, WhenPageBoundaryIsCrossedSecondPartIsWritten)
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

TEST_P(FlashSpiTest, EraseFirstSubSector)
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

TEST_P(FlashSpiTest, EraseFirstSubSectorFinishesOnFlagPoll)
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

TEST_P(FlashSpiTest, EraseSecondSubSector)
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

TEST_P(FlashSpiTest, EraseSubSectorFromAddress)
{
    auto address = flash.AddressOfSector(flash.SectorOfAddress(GetParam().address));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSubSector, address), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flash.EraseSector(flash.SectorOfAddress(GetParam().address), [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

TEST_P(FlashSpiTest, EraseMultipleErasesOneSubSector)
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

TEST_P(FlashSpiTest, EraseMultipleErasesTwoSubSectors)
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

TEST_P(FlashSpiTest, EraseMultipleErasesSector)
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

TEST_P(FlashSpiTest, EraseMultipleErasesSubSectorsAndSector)
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

TEST_P(FlashSpiTest, EraseAllErasesBulk)
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

TEST_P(FlashSpiTest, EraseSubSectorWhenSubSectorEqualsSector)
{
    services::FlashSpi::Config config;
    config.nrOfSubSectors = 64;
    config.sizeSector = 1024;
    config.sizeSubSector = 1024;
    config.sizePage = 256;
    config.extendedAddressing = GetParam().extendedAddressing;
    services::FlashSpi flashConfigured{ spiMock, config };

    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandWriteEnable), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstructionAndAddress(services::FlashSpi::commandEraseSector, 0), hal::SpiAction::stop));
    EXPECT_CALL(spiMock, SendDataMock(CreateInstruction(services::FlashSpi::commandReadStatusRegister), hal::SpiAction::continueSession));
    EXPECT_CALL(spiMock, ReceiveDataMock(hal::SpiAction::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 1 }));

    flashConfigured.EraseSectors(0, 1, [this]()
        {
            finished.callback();
        });
    ExecuteAllActions();
}

INSTANTIATE_TEST_SUITE_P(FlashSpiTests, FlashSpiTest,
    testing::Values(
        FlashSpiTestArguments{ false, 0, 512 },
        FlashSpiTestArguments{ false, 0x123456, 512 },
        FlashSpiTestArguments{ true, 0, 16384 },
        FlashSpiTestArguments{ true, 0x1000000, 16384 }));
