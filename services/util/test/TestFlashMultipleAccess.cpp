#include "gtest/gtest.h"
#include "hal/interfaces/test_doubles/FlashMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "services/util/FlashMultipleAccess.hpp"

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

    testing::StrictMock<hal::FlashMock> flash;
    services::FlashMultipleAccessMaster multipleAccess;
    services::FlashMultipleAccess access1;
    services::FlashMultipleAccess access2;
};

TEST_F(FlashMultipleAccessTest, Construction)
{}

TEST_F(FlashMultipleAccessTest, FirstReadIsExecuted)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(flash, readBufferMock(testing::_)).WillOnce(testing::Return(std::vector<uint8_t>(1, 0)));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondReadIsNotExecuted)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(flash, readBufferMock(testing::_)).WillOnce(testing::Return(std::vector<uint8_t>(1, 0)));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.ReadBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondReadIsExecutedWhenFirstAccessFinishes)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(flash, readBufferMock(testing::_)).WillOnce(testing::Return(std::vector<uint8_t>(1, 0)));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.ReadBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_CALL(flash, readBufferMock(testing::_)).WillOnce(testing::Return(std::vector<uint8_t>(1, 0)));
    flash.done();
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, FirstWriteIsExecuted)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(flash, writeBufferMock(testing::_, testing::_));
    access1.WriteBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondWriteIsNotExecuted)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(flash, readBufferMock(testing::_)).WillOnce(testing::Return(std::vector<uint8_t>(1, 0)));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.WriteBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondWriteIsExecutedWhenFirstAccessFinishes)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(flash, readBufferMock(testing::_)).WillOnce(testing::Return(std::vector<uint8_t>(1, 0)));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.WriteBuffer(buffer, 0, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_CALL(flash, writeBufferMock(testing::_, testing::_));
    flash.done();
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, FirstEraseIsExecuted)
{
    EXPECT_CALL(flash, eraseSectorsMock(testing::_, testing::_));
    access1.EraseSectors(0, 1, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondEraseIsNotExecuted)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(flash, readBufferMock(testing::_)).WillOnce(testing::Return(std::vector<uint8_t>(1, 0)));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.EraseSectors(0, 1, infra::emptyFunction);
    ExecuteAllActions();
}

TEST_F(FlashMultipleAccessTest, SecondEraseIsExecutedWhenFirstAccessFinishes)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(flash, readBufferMock(testing::_)).WillOnce(testing::Return(std::vector<uint8_t>(1, 0)));
    access1.ReadBuffer(buffer, 0, infra::emptyFunction);
    access2.EraseSectors(0, 1, infra::emptyFunction);
    ExecuteAllActions();

    EXPECT_CALL(flash, eraseSectorsMock(testing::_, testing::_));
    flash.done();
    ExecuteAllActions();
}
