#include "gmock/gmock.h"
#include "hal/interfaces/I2cRegisterAccess.hpp"
#include "hal/interfaces/test_doubles/I2cMock.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include <vector>

class I2cRegisterAccessTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    static const hal::I2cAddress slaveAddress;

    I2cRegisterAccessTest()
        : master()
        , registerAccess(master, slaveAddress)
    {}

    hal::I2cMasterMock master;
    hal::I2cMasterRegisterAccessByte registerAccess;
};

const hal::I2cAddress I2cRegisterAccessTest::slaveAddress(0x5a);

TEST_F(I2cRegisterAccessTest, TestReadRegister)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback());
    EXPECT_CALL(master, SendDataMock(slaveAddress, hal::Action::repeatedStart, std::vector<uint8_t>{ 3 }));
    EXPECT_CALL(master, ReceiveDataMock(slaveAddress, hal::Action::stop)).WillOnce(testing::Return(std::vector<uint8_t>{ 5, 6, 7 }));

    std::array<uint8_t, 3> data;
    registerAccess.ReadRegister(3, infra::MakeByteRange(data), [&callback]() { callback.callback(); });

    ExecuteAllActions();

    EXPECT_EQ((std::array<uint8_t, 3>{ 5, 6, 7 }), data);
}

TEST_F(I2cRegisterAccessTest, TestWriteRegister)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback());
    EXPECT_CALL(master, SendDataMock(slaveAddress, hal::Action::continueSession, std::vector<uint8_t>{ 3 }));
    EXPECT_CALL(master, SendDataMock(slaveAddress, hal::Action::stop, std::vector<uint8_t>{ 5, 6, 7 }));

    std::array<uint8_t, 3> data{ 5, 6, 7 };
    registerAccess.WriteRegister(3, infra::MakeByteRange(data), [&callback]() { callback.callback(); });

    ExecuteAllActions();
}
