#include "hal/interfaces/test_doubles/I2cRegisterAccessMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "gtest/gtest.h"

class RegisterAccessMockTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    RegisterAccessMockTest()
        : master(masterMock)
    {}

    hal::I2cMasterRegisterAccessMock masterMock;
    hal::I2cMaster& master;
};

TEST_F(RegisterAccessMockTest, ReadRegisterInOneRead)
{
    infra::VerifyingFunction<void(hal::Result, uint32_t)> sendDataDoneVerifier(hal::Result::complete, 1);
    infra::VerifyingFunction<void(hal::Result)> receiveDataDoneVerifier(hal::Result::complete);
    EXPECT_CALL(masterMock, ReadRegisterMock(3)).WillOnce(testing::Return(std::vector<uint8_t>{ 1, 2, 3, 4 }));

    uint8_t dataRegister = 3;
    std::array<uint8_t, 4> data;
    master.SendData(hal::I2cAddress(1), infra::MakeByteRange(dataRegister), hal::Action::repeatedStart, sendDataDoneVerifier);
    master.ReceiveData(hal::I2cAddress(1), infra::MakeByteRange(data), hal::Action::stop, receiveDataDoneVerifier);

    EXPECT_EQ((std::array<uint8_t, 4>{ 1, 2, 3, 4 }), data);
}

TEST_F(RegisterAccessMockTest, ReadRegisterInTwoReads)
{
    infra::VerifyingFunction<void(hal::Result)> receiveDataDone1Verifier(hal::Result::complete);
    infra::VerifyingFunction<void(hal::Result)> receiveDataDone2Verifier(hal::Result::complete);
    EXPECT_CALL(masterMock, ReadRegisterMock(3)).WillOnce(testing::Return(std::vector<uint8_t>{ 1, 2, 3, 4 }));

    uint8_t dataRegister = 3;
    master.SendData(hal::I2cAddress(1), infra::MakeByteRange(dataRegister), hal::Action::repeatedStart, [](hal::Result, uint32_t numberOfBytesSent) {});

    std::array<uint8_t, 3> data1;
    std::array<uint8_t, 1> data2;
    master.ReceiveData(hal::I2cAddress(1), infra::MakeByteRange(data1), hal::Action::continueSession, receiveDataDone1Verifier);
    master.ReceiveData(hal::I2cAddress(1), infra::MakeByteRange(data2), hal::Action::stop, receiveDataDone2Verifier);
    EXPECT_EQ((std::array<uint8_t, 3>{ 1, 2, 3 }), data1);
    EXPECT_EQ((std::array<uint8_t, 1>{ 4 }), data2);
}

TEST_F(RegisterAccessMockTest, WriteRegisterInOneWrite)
{
    infra::VerifyingFunction<void(hal::Result, uint32_t)> sendDataDoneVerifier(hal::Result::complete, 5);
    EXPECT_CALL(masterMock, WriteRegisterMock(3, std::vector<uint8_t>{ 1, 2, 3, 4 }));

    std::array<uint8_t, 5> data = { 3, 1, 2, 3, 4 };
    master.SendData(hal::I2cAddress(1), infra::MakeByteRange(data), hal::Action::stop, sendDataDoneVerifier);
}

TEST_F(RegisterAccessMockTest, WriteRegisterInTwoWrites)
{
    infra::VerifyingFunction<void(hal::Result, uint32_t)> sendDataDone1Verifier(hal::Result::complete, 1);
    infra::VerifyingFunction<void(hal::Result, uint32_t)> sendDataDone2Verifier(hal::Result::complete, 4);
    EXPECT_CALL(masterMock, WriteRegisterMock(3, std::vector<uint8_t>{ 1, 2, 3, 4 }));

    uint8_t dataRegister = 3;
    std::array<uint8_t, 4> data = { 1, 2, 3, 4 };
    master.SendData(hal::I2cAddress(1), infra::MakeByteRange(dataRegister), hal::Action::continueSession, sendDataDone1Verifier);
    master.SendData(hal::I2cAddress(1), infra::MakeByteRange(data), hal::Action::stop, sendDataDone2Verifier);
}

TEST_F(RegisterAccessMockTest, WriteRegisterInThreeWrites)
{
    infra::VerifyingFunction<void(hal::Result, uint32_t)> sendDataDone1Verifier(hal::Result::complete, 1);
    infra::VerifyingFunction<void(hal::Result, uint32_t)> sendDataDone2Verifier(hal::Result::complete, 3);
    infra::VerifyingFunction<void(hal::Result, uint32_t)> sendDataDone3Verifier(hal::Result::complete, 1);
    EXPECT_CALL(masterMock, WriteRegisterMock(3, std::vector<uint8_t>{ 1, 2, 3, 4 }));

    uint8_t dataRegister = 3;
    std::array<uint8_t, 3> data1 = { 1, 2, 3 };
    std::array<uint8_t, 1> data2 = { 4 };
    master.SendData(hal::I2cAddress(1), infra::MakeByteRange(dataRegister), hal::Action::continueSession, sendDataDone1Verifier);
    master.SendData(hal::I2cAddress(1), infra::MakeByteRange(data1), hal::Action::continueSession, sendDataDone2Verifier);
    master.SendData(hal::I2cAddress(1), infra::MakeByteRange(data2), hal::Action::stop, sendDataDone3Verifier);
}
