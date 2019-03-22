#include "gmock/gmock.h"
#include "hal/interfaces/test_doubles/QuadSpiStub.hpp"
#include "infra/timer/test_helper/ClockFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/FlashQuadSpiCypressFll.hpp"

class FlashQuadSpiCypressFllTest
    : public testing::Test
    , public infra::ClockFixture
{
public:
    FlashQuadSpiCypressFllTest()
        : flash(spiStub, onInitialized)
    {
        EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEnterQpi), {}, {}, 0 },
            infra::ConstByteRange(), hal::QuadSpi::Lines::SingleSpeed()));

        ForwardTime(std::chrono::milliseconds(100));
        testing::Mock::VerifyAndClear(&spiStub);
        testing::Mock::VerifyAndClear(&onInitialized);
    }

    testing::StrictMock<hal::QuadSpiStub> spiStub;
    infra::VerifyingFunctionMock<void()> onInitialized;
    services::FlashQuadSpiCypressFll flash;

    testing::StrictMock<infra::MockCallback<void()>> finished;
};

#define EXPECT_ENABLE_WRITE() EXPECT_CALL(spiStub, SendDataMock( \
    hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandWriteEnable), {}, {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()))
#define EXPECT_POLL_WRITE_DONE() EXPECT_CALL(spiStub, PollStatusMock( \
    hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandReadStatusRegister), {}, {}, 0 }, 1, 0, 1, hal::QuadSpi::Lines::QuadSpeed()))

TEST_F(FlashQuadSpiCypressFllTest, Construction)
{
    EXPECT_EQ(4096, flash.NumberOfSectors());
    EXPECT_EQ(4096, flash.SizeOfSector(0));
}

TEST_F(FlashQuadSpiCypressFllTest, ReadData)
{
    std::array<uint8_t, 4> receiveData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiStub, ReceiveDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandReadData), hal::QuadSpi::AddressToVector(0, 4), {}, 8 }, hal::QuadSpi::Lines::QuadSpeed()))
        .WillOnce(testing::Return(infra::MakeByteRange(receiveData)));
    EXPECT_CALL(finished, callback());

    std::array<uint8_t, 4> buffer;
    flash.ReadBuffer(buffer, 0, [this]() { finished.callback(); });
    ExecuteAllActions();
    
    EXPECT_EQ(receiveData, buffer);
}

TEST_F(FlashQuadSpiCypressFllTest, ReadDataAtNonZeroAddress)
{
    std::array<uint8_t, 4> receiveData = { 1, 2, 3, 4 };
    EXPECT_CALL(spiStub, ReceiveDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandReadData), hal::QuadSpi::AddressToVector(0x123456 << 8, 4), {}, 8 }, hal::QuadSpi::Lines::QuadSpeed()))
        .WillOnce(testing::Return(infra::MakeByteRange(receiveData)));
    EXPECT_CALL(finished, callback());

    std::array<uint8_t, 4> buffer;
    flash.ReadBuffer(buffer, 0 + 0x123456, [this]() { finished.callback(); });
    ExecuteAllActions();
    
    EXPECT_EQ(receiveData, buffer);
}

TEST_F(FlashQuadSpiCypressFllTest, WriteData)
{
    const std::array<uint8_t, 4> sendData = { 1, 2, 3, 4 };
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandPageProgram), hal::QuadSpi::AddressToVector(0, 3), {}, 0 }, infra::MakeByteRange(sendData), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.WriteBuffer(sendData, 0, [this]() { finished.callback(); });
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, WriteDataFinishesOnFlagPoll)
{
    const std::array<uint8_t, 4> sendData = { 1, 2, 3, 4 };
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandPageProgram), hal::QuadSpi::AddressToVector(0, 3), {}, 0 }, infra::MakeByteRange(sendData), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.WriteBuffer(sendData, 0, [this]() { finished.callback(); });
    ExecuteAllActions();

    EXPECT_CALL(finished, callback());
    spiStub.onDone();
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, WriteDataAtNonZeroAddress)
{
    const std::array<uint8_t, 4> sendData = { 1, 2, 3, 4 };
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandPageProgram), hal::QuadSpi::AddressToVector(0x123456, 3), {}, 0 }, infra::MakeByteRange(sendData), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.WriteBuffer(sendData, 0 + 0x123456, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, WhenPageBoundaryIsCrossedFirstPartIsWritten)
{
    const std::array<uint8_t, 4> sendData = { 1, 2, 3, 4 };
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandPageProgram), hal::QuadSpi::AddressToVector(254, 3), {}, 0 }, infra::MakeRange(sendData.data(), sendData.data() + 2), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.WriteBuffer(sendData, 0 + 254, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, WhenPageBoundaryIsCrossedSecondPartIsWritten)
{
    testing::InSequence s;

    const std::array<uint8_t, 6> sendData = { 1, 2, 3, 4, 5, 6 };
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandPageProgram), hal::QuadSpi::AddressToVector(254, 3), {}, 0 }, infra::MakeRange(sendData.data(), sendData.data() + 2), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandPageProgram), hal::QuadSpi::AddressToVector(256, 3), {}, 0 }, infra::MakeRange(sendData.data() + 2, sendData.data() + 6), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.WriteBuffer(sendData, 0 + 254, infra::emptyFunction);
    spiStub.onDone();
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, EraseFirstSector)
{
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseSector), hal::QuadSpi::AddressToVector(0, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.EraseSector(0, [this]() { finished.callback(); });
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, EraseFirstSectorFinishesOnFlagPoll)
{
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseSector), hal::QuadSpi::AddressToVector(0, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.EraseSector(0, [this]() { finished.callback(); });
    ExecuteAllActions();

    EXPECT_CALL(finished, callback());
    spiStub.onDone();
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, EraseSecondSector)
{
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseSector), hal::QuadSpi::AddressToVector(4096, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.EraseSector(1, [this]() { finished.callback(); });
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, EraseMultipleErasesSubSector)
{
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseSector), hal::QuadSpi::AddressToVector(0, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.EraseSectors(0, 1, [this]() { finished.callback(); });
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, EraseMultipleErasesTwoSectors)
{
    testing::InSequence s;

    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseSector), hal::QuadSpi::AddressToVector(0, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseSector), hal::QuadSpi::AddressToVector(4096, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.EraseSectors(0, 2, [this]() { finished.callback(); });
    spiStub.onDone();
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, EraseMultipleErasesBlock)
{
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseBlock), hal::QuadSpi::AddressToVector(0, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.EraseSectors(0, 16, [this]() { finished.callback(); });
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, EraseMultipleErasesSectorsAndBlock)
{
    testing::InSequence s;
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseSector), hal::QuadSpi::AddressToVector(61440, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseBlock), hal::QuadSpi::AddressToVector(65536, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseSector), hal::QuadSpi::AddressToVector(131072, 3), {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.EraseSectors(15, 33, [this]() { finished.callback(); });
    spiStub.onDone();
    spiStub.onDone();
    ExecuteAllActions();
}

TEST_F(FlashQuadSpiCypressFllTest, EraseAllErasesChip)
{
    EXPECT_ENABLE_WRITE();
    EXPECT_CALL(spiStub, SendDataMock(hal::QuadSpi::Header{ infra::MakeOptional(services::FlashQuadSpiCypressFll::commandEraseChip), {}, {}, 0 }, infra::ConstByteRange(), hal::QuadSpi::Lines::QuadSpeed()));
    EXPECT_POLL_WRITE_DONE();

    flash.EraseAll([this]() { finished.callback(); });
    ExecuteAllActions();
}
