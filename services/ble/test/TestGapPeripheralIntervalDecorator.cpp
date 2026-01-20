#include "services/ble/GapPeripheralIntervalDecorator.hpp"
#include "services/ble/test_doubles/GapPeripheralMock.hpp"
#include "services/ble/test_doubles/GapPeripheralObserverMock.hpp"
#include "gmock/gmock.h"

class GapPeripheralIntervalDecoratorTest
    : public testing::Test
{
public:
    testing::StrictMock<services::GapPeripheralMock> gap;
    services::GapPeripheralIntervalDecorator gapPeripheralIntervalDecorator{ gap };
    testing::StrictMock<services::GapPeripheralObserverMock> gapObserver{ gapPeripheralIntervalDecorator };
};

TEST_F(GapPeripheralIntervalDecoratorTest, use_user_adv_interval)
{
    EXPECT_CALL(gap, Advertise(services::GapAdvertisementType::advInd, 10));
    gapPeripheralIntervalDecorator.Advertise(services::GapAdvertisementType::advInd, 10);
}

TEST_F(GapPeripheralIntervalDecoratorTest, use_long_adv_interval)
{
    gapPeripheralIntervalDecorator.SwitchToLongInterval();
    EXPECT_CALL(gap, Advertise(services::GapAdvertisementType::advInd, 50));
    gapPeripheralIntervalDecorator.Advertise(services::GapAdvertisementType::advInd, 10);
}

TEST_F(GapPeripheralIntervalDecoratorTest, switch_to_long_and_back_interval_while_advertise)
{
    EXPECT_CALL(gap, Advertise(services::GapAdvertisementType::advInd, 10));
    gapPeripheralIntervalDecorator.Advertise(services::GapAdvertisementType::advInd, 10);

    EXPECT_CALL(gapObserver, StateChanged(services::GapState::advertising));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::advertising);

    EXPECT_CALL(gap, Standby());
    gapPeripheralIntervalDecorator.SwitchToLongInterval();

    EXPECT_CALL(gapObserver, StateChanged(services::GapState::standby));
    EXPECT_CALL(gap, Advertise(services::GapAdvertisementType::advInd, 50));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::standby);

    EXPECT_CALL(gapObserver, StateChanged(services::GapState::advertising));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::advertising);

    EXPECT_CALL(gap, Standby());
    gapPeripheralIntervalDecorator.SwitchToUserInterval();

    EXPECT_CALL(gapObserver, StateChanged(services::GapState::standby));
    EXPECT_CALL(gap, Advertise(services::GapAdvertisementType::advInd, 10));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::standby);
}

TEST_F(GapPeripheralIntervalDecoratorTest, requested_standby_will_overwrite_pending_adv)
{
    EXPECT_CALL(gap, Advertise(services::GapAdvertisementType::advInd, 10));
    gapPeripheralIntervalDecorator.Advertise(services::GapAdvertisementType::advInd, 10);

    EXPECT_CALL(gapObserver, StateChanged(services::GapState::advertising));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::advertising);

    EXPECT_CALL(gap, Standby());
    gapPeripheralIntervalDecorator.Standby();

    EXPECT_CALL(gapObserver, StateChanged(services::GapState::standby));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::standby);
}

TEST_F(GapPeripheralIntervalDecoratorTest, use_default_connection_parameter_when_connect)
{
    EXPECT_CALL(gapObserver, StateChanged(services::GapState::connected));
    EXPECT_CALL(gap, SetConnectionParameters(testing::_)).WillOnce(testing::Invoke([](const services::GapConnectionParameters& connParam)
        {
            EXPECT_EQ(connParam.minConnIntMultiplier, 6);
            EXPECT_EQ(connParam.maxConnIntMultiplier, 6);
            EXPECT_EQ(connParam.slaveLatency, 0);
            EXPECT_EQ(connParam.supervisorTimeoutMs, 500);
        }));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::connected);
}

TEST_F(GapPeripheralIntervalDecoratorTest, use_user_connection_parameter_when_connect)
{
    services::GapConnectionParameters connParam{ 10, 10, 0, 500 };
    gapPeripheralIntervalDecorator.SetConnectionParameters(connParam);

    EXPECT_CALL(gapObserver, StateChanged(services::GapState::connected));
    EXPECT_CALL(gap, SetConnectionParameters(testing::_)).WillOnce(testing::Invoke([](const services::GapConnectionParameters& connParam)
        {
            EXPECT_EQ(connParam.minConnIntMultiplier, 10);
            EXPECT_EQ(connParam.maxConnIntMultiplier, 10);
            EXPECT_EQ(connParam.slaveLatency, 0);
            EXPECT_EQ(connParam.supervisorTimeoutMs, 500);
        }));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::connected);
}

TEST_F(GapPeripheralIntervalDecoratorTest, use_long_connection_parameter_when_connect)
{
    gapPeripheralIntervalDecorator.SwitchToLongInterval();

    EXPECT_CALL(gapObserver, StateChanged(services::GapState::connected));
    EXPECT_CALL(gap, SetConnectionParameters(testing::_)).WillOnce(testing::Invoke([](const services::GapConnectionParameters& connParam)
        {
            EXPECT_EQ(connParam.minConnIntMultiplier, 50);
            EXPECT_EQ(connParam.maxConnIntMultiplier, 50);
            EXPECT_EQ(connParam.slaveLatency, 0);
            EXPECT_EQ(connParam.supervisorTimeoutMs, 500);
        }));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::connected);
}

TEST_F(GapPeripheralIntervalDecoratorTest, switch_to_long_connection_interval_and_back)
{
    EXPECT_CALL(gapObserver, StateChanged(services::GapState::connected));
    EXPECT_CALL(gap, SetConnectionParameters(testing::_)).WillOnce(testing::Invoke([](const services::GapConnectionParameters& connParam)
        {
            EXPECT_EQ(connParam.minConnIntMultiplier, 6);
            EXPECT_EQ(connParam.maxConnIntMultiplier, 6);
            EXPECT_EQ(connParam.slaveLatency, 0);
            EXPECT_EQ(connParam.supervisorTimeoutMs, 500);
        }));
    gapPeripheralIntervalDecorator.StateChanged(services::GapState::connected);

    EXPECT_CALL(gap, SetConnectionParameters(testing::_)).WillOnce(testing::Invoke([](const services::GapConnectionParameters& connParam)
        {
            EXPECT_EQ(connParam.minConnIntMultiplier, 50);
            EXPECT_EQ(connParam.maxConnIntMultiplier, 50);
            EXPECT_EQ(connParam.slaveLatency, 0);
            EXPECT_EQ(connParam.supervisorTimeoutMs, 500);
        }));
    gapPeripheralIntervalDecorator.SwitchToLongInterval();

    EXPECT_CALL(gap, SetConnectionParameters(testing::_)).WillOnce(testing::Invoke([](const services::GapConnectionParameters& connParam)
        {
            EXPECT_EQ(connParam.minConnIntMultiplier, 6);
            EXPECT_EQ(connParam.maxConnIntMultiplier, 6);
            EXPECT_EQ(connParam.slaveLatency, 0);
            EXPECT_EQ(connParam.supervisorTimeoutMs, 500);
        }));
    gapPeripheralIntervalDecorator.SwitchToUserInterval();
}
