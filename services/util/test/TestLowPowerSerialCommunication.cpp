#include "hal/interfaces/test_doubles/SerialCommunicationMock.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/util/LowPowerSerialCommunication.hpp"
#include "gtest/gtest.h"

class LowPowerSerialCommunicationTest
    : public testing::Test
{
public:
    LowPowerSerialCommunicationTest()
        : lowPowerSerialCommunication(serialComm, mainClock)
    {}

    testing::StrictMock<hal::SerialCommunicationMock> serialComm;
    infra::MainClockReference mainClock;
    hal::LowPowerSerialCommunication lowPowerSerialCommunication;
};

TEST_F(LowPowerSerialCommunicationTest, construction)
{
    EXPECT_FALSE(mainClock.IsReferenced());
}

TEST_F(LowPowerSerialCommunicationTest, SendData_should_acquire_mainClock_and_release_in_actionOnCompletion)
{
    infra::MockCallback<void()> mockActionOnCompletion;

    EXPECT_CALL(serialComm, SendDataMock(std::vector<uint8_t>{ 0xfe, 0xff }));
    lowPowerSerialCommunication.SendData(std::vector<uint8_t>{ 0xfe, 0xff }, [&mockActionOnCompletion]()
        { mockActionOnCompletion.callback(); });
    EXPECT_TRUE(mainClock.IsReferenced());

    EXPECT_CALL(mockActionOnCompletion, callback());
    serialComm.actionOnCompletion();
    EXPECT_FALSE(mainClock.IsReferenced());
}

TEST_F(LowPowerSerialCommunicationTest, ReceiveData_should_forward_callback_registration_to_downStream)
{
    infra::ConstByteRange savedData;
    lowPowerSerialCommunication.ReceiveData([&savedData](infra::ConstByteRange data)
        { savedData = data; });

    std::vector<uint8_t> data = { 0xfe, 0xff, 0xee };
    serialComm.dataReceived(data);
    EXPECT_EQ(savedData, data);
}
