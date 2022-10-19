#include "hal/interfaces/test_doubles/I2cMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "services/util/I2cMultipleAccess.hpp"
#include "gtest/gtest.h"

class I2cMultipleAccessTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    I2cMultipleAccessTest()
        : multipleAccess(i2c)
        , access1(multipleAccess)
        , access2(multipleAccess)
    {}

    testing::StrictMock<hal::I2cMasterMockWithoutAutomaticDone> i2c;
    services::I2cMultipleAccessMaster multipleAccess;
    services::I2cMultipleAccess access1;
    services::I2cMultipleAccess access2;
};

TEST_F(I2cMultipleAccessTest, FirstReceiveIsExecuted)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(i2c, ReceiveDataMock(testing::_, hal::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 5 }));
    access1.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, SecondReceiveIsNotExecuted)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(i2c, ReceiveDataMock(testing::_, hal::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 5 }));
    access1.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    access2.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, SecondReceiveIsExecutedWhenFirstAccessFinishes)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(i2c, ReceiveDataMock(testing::_, hal::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 5 }));
    access1.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    access2.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    ExecuteAllActions();

    EXPECT_CALL(i2c, ReceiveDataMock(testing::_, hal::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 5 }));
    i2c.onReceived(hal::Result::complete);
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, AfterReceiveWithRepeatedStartClaimIsNotReleased)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(i2c, ReceiveDataMock(testing::_, hal::Action::repeatedStart)).WillOnce(testing::Return(std::vector<uint8_t>{ 5 }));
    access1.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::repeatedStart, [](hal::Result) {});
    access2.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    ExecuteAllActions();

    i2c.onReceived(hal::Result::complete);
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, AfterReceiveWithContinueSessionClaimIsNotReleased)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(i2c, ReceiveDataMock(testing::_, hal::Action::continueSession)).WillOnce(testing::Return(std::vector<uint8_t>{ 5 }));
    access1.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::continueSession, [](hal::Result) {});
    access2.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    ExecuteAllActions();

    i2c.onReceived(hal::Result::complete);
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, ReceiveStopAfterRepeatedStartReleasesTheClaim)
{
    std::array<uint8_t, 1> buffer;

    EXPECT_CALL(i2c, ReceiveDataMock(testing::_, hal::Action::repeatedStart)).WillOnce(testing::Return(std::vector<uint8_t>{ 5 }));
    access1.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::repeatedStart, [](hal::Result) {});
    access2.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    ExecuteAllActions();

    i2c.onReceived(hal::Result::complete);
    ExecuteAllActions();

    testing::Mock::VerifyAndClearExpectations(&i2c);

    EXPECT_CALL(i2c, ReceiveDataMock(testing::_, hal::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 5 })).WillOnce(testing::Return(std::vector<uint8_t>{ 5 }));
    access1.ReceiveData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result) {});
    ExecuteAllActions();
    i2c.onReceived(hal::Result::complete);
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, FirstSendIsExecuted)
{
    std::array<uint8_t, 1> buffer = { 5 };

    EXPECT_CALL(i2c, SendDataMock(testing::_, hal::Action::stop, std::vector<uint8_t>{ 5 }));
    access1.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, SecondSendIsNotExecuted)
{
    std::array<uint8_t, 1> buffer = { 5 };

    EXPECT_CALL(i2c, SendDataMock(testing::_, hal::Action::stop, std::vector<uint8_t>{ 5 }));
    access1.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    access2.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, SecondSendIsExecutedWhenFirstAccessFinishes)
{
    std::array<uint8_t, 1> buffer = { 5 };

    EXPECT_CALL(i2c, SendDataMock(testing::_, hal::Action::stop, std::vector<uint8_t>{ 5 }));
    access1.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    access2.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    ExecuteAllActions();

    EXPECT_CALL(i2c, SendDataMock(testing::_, hal::Action::stop, std::vector<uint8_t>{ 5 }));
    i2c.onSent(hal::Result::complete, 1);
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, AfterSendWithRepeatedStartClaimIsNotReleased)
{
    std::array<uint8_t, 1> buffer = { 5 };

    EXPECT_CALL(i2c, SendDataMock(testing::_, hal::Action::repeatedStart, std::vector<uint8_t>{ 5 }));
    access1.SendData(hal::I2cAddress(1), buffer, hal::Action::repeatedStart, [](hal::Result, uint32_t) {});
    access2.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    ExecuteAllActions();

    i2c.onSent(hal::Result::complete, 1);
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, AfterSendWithContinueSessionClaimIsNotReleased)
{
    std::array<uint8_t, 1> buffer = { 5 };

    EXPECT_CALL(i2c, SendDataMock(testing::_, hal::Action::continueSession, std::vector<uint8_t>{ 5 }));
    access1.SendData(hal::I2cAddress(1), buffer, hal::Action::continueSession, [](hal::Result, uint32_t) {});
    access2.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    ExecuteAllActions();

    i2c.onSent(hal::Result::complete, 1);
    ExecuteAllActions();
}

TEST_F(I2cMultipleAccessTest, SendStopAfterRepeatedStartReleasesTheClaim)
{
    std::array<uint8_t, 1> buffer = { 5 };

    EXPECT_CALL(i2c, SendDataMock(testing::_, hal::Action::repeatedStart, std::vector<uint8_t>{ 5 }));
    access1.SendData(hal::I2cAddress(1), buffer, hal::Action::repeatedStart, [](hal::Result, uint32_t) {});
    access2.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    ExecuteAllActions();

    i2c.onSent(hal::Result::complete, 1);
    ExecuteAllActions();

    testing::Mock::VerifyAndClearExpectations(&i2c);

    EXPECT_CALL(i2c, SendDataMock(testing::_, hal::Action::stop, std::vector<uint8_t>{ 5 })).Times(2);
    access1.SendData(hal::I2cAddress(1), buffer, hal::Action::stop, [](hal::Result, uint32_t) {});
    ExecuteAllActions();
    i2c.onSent(hal::Result::complete, 1);
    ExecuteAllActions();
}
