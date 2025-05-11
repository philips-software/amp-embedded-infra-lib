#include "hal/interfaces/test_doubles/FlashMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "services/util/FlashMultipleAccess.hpp"
#include "gtest/gtest.h"

class FlashMultipleAccessTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    FlashMultipleAccessTest()
        : multipleAccess(flash)
        , access1(multipleAccess)
        , access2(multipleAccess)
    {}

    testing::StrictMock<hal::CleanFlashMock> flash;
    services::FlashMultipleAccessMaster multipleAccess;
    services::FlashMultipleAccess access1;
    services::FlashMultipleAccess access2;

    std::array<uint8_t, 1> buffer;
    infra::Function<void()> onDone;
};

TEST_F(FlashMultipleAccessTest, Construction)
{
    EXPECT_CALL(flash, NumberOfSectors()).WillOnce(testing::Return(1));
    EXPECT_EQ(1, access1.NumberOfSectors());

    EXPECT_CALL(flash, SizeOfSector(2)).WillOnce(testing::Return(1));
    EXPECT_EQ(1, access1.SizeOfSector(2));

    EXPECT_CALL(flash, SectorOfAddress(2)).WillOnce(testing::Return(1));
    EXPECT_EQ(1, access1.SectorOfAddress(2));

    EXPECT_CALL(flash, AddressOfSector(2)).WillOnce(testing::Return(1));
    EXPECT_EQ(1, access1.AddressOfSector(2));
}

TEST_F(FlashMultipleAccessTest, FirstReadIsExecuted)
{
    EXPECT_CALL(flash, ReadBuffer(testing::_, 0, testing::_));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondReadIsNotExecuted)
{
    EXPECT_CALL(flash, ReadBuffer(testing::_, 0, testing::_));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.ReadBuffer(buffer, 1, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondReadIsExecutedWhenFirstAccessFinishes)
{
    EXPECT_CALL(flash, ReadBuffer(infra::MakeRange(buffer), 0, testing::_)).WillOnce(testing::SaveArg<2>(&onDone));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.ReadBuffer(buffer, 1, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_CALL(flash, ReadBuffer(infra::MakeRange(buffer), 1, testing::_));
    onDone();
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, FirstWriteIsExecuted)
{
    EXPECT_CALL(flash, WriteBuffer(infra::MakeConst(infra::MakeRange(buffer)), 0, testing::_));
    access1.WriteBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondWriteIsNotExecuted)
{
    EXPECT_CALL(flash, ReadBuffer(testing::_, 0, testing::_));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.WriteBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondWriteIsExecutedWhenFirstAccessFinishes)
{
    EXPECT_CALL(flash, ReadBuffer(infra::MakeRange(buffer), 0, testing::_)).WillOnce(testing::SaveArg<2>(&onDone));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.WriteBuffer(buffer, 1, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_CALL(flash, WriteBuffer(infra::MakeConst(infra::MakeRange(buffer)), 1, testing::_));
    onDone();
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, FirstEraseIsExecuted)
{
    EXPECT_CALL(flash, EraseSectors(0, 1, testing::_));
    access1.EraseSectors(0, 1, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondEraseIsNotExecuted)
{
    EXPECT_CALL(flash, ReadBuffer(infra::MakeRange(buffer), 0, testing::_));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.EraseSectors(0, 1, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondEraseIsExecutedWhenFirstAccessFinishes)
{
    EXPECT_CALL(flash, ReadBuffer(infra::MakeRange(buffer), 0, testing::_)).WillOnce(testing::SaveArg<2>(&onDone));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.EraseSectors(0, 1, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_CALL(flash, EraseSectors(0, 1, testing::_));
    onDone();
    ExecuteAllActions();
}
