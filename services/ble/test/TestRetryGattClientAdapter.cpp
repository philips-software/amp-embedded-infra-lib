#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/GattClient.hpp"
#include "services/ble/RetryGattClientCharacteristicsOperations.hpp"
#include "services/ble/test_doubles/ClaimingGattClientAdapterMock.hpp"
#include "services/ble/test_doubles/GapCentralMock.hpp"
#include "services/ble/test_doubles/GattClientMock.hpp"
#include "services/ble/test_doubles/GattMock.hpp"
#include "gmock/gmock.h"

namespace
{
    class RetryGattClientCharacteristicsOperationsTest
        : public testing::Test
        , public infra::EventDispatcherFixture
    {
    public:
        services::GattClientMock gattClient;
        services::AttMtuExchangeMock attMtuExchange;
        services::GapCentralMock gapCentral;
        testing::StrictMock<services::ClaimingGattClientAdapterMock> adapter{ gattClient, attMtuExchange, gapCentral };
        testing::StrictMock<services::RetryGattClientCharacteristicsOperations> retryAdapter{ adapter };
        testing::StrictMock<services::GattClientStackUpdateObserverMock> stackUpdateObserver{ retryAdapter };

        static constexpr uint16_t handle = 0x1;
        std::array<uint8_t, 4> data{ 0x01, 0x02, 0x03, 0x04 };
        infra::Function<void(services::OperationStatus)> onDone;
    };
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_read_characteristic)
{
    infra::Function<void(const infra::ConstByteRange&)> onReadMock;

    EXPECT_CALL(adapter, Read(handle, testing::Ref(onReadMock), testing::Ref(onDone)));

    retryAdapter.Read(handle, onReadMock, onDone);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_write_characteristic)
{
    EXPECT_CALL(adapter, Write(handle, testing::ElementsAreArray(data), testing::Ref(onDone)));

    retryAdapter.Write(handle, data, onDone);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_write_without_response_characteristic_and_error)
{
    infra::VerifyingFunction<void(services::OperationStatus)> errorCallback(services::OperationStatus::error);

    EXPECT_CALL(adapter, WriteWithoutResponse(handle, testing::ElementsAreArray(data), testing::_))
        .WillOnce(testing::InvokeArgument<2>(services::OperationStatus::error));

    retryAdapter.WriteWithoutResponse(handle, data, errorCallback);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_write_without_response_characteristic_with_retry)
{
    testing::StrictMock<infra::MockCallback<void(services::OperationStatus)>> callback;
    infra::Function<void(services::OperationStatus)> onDone;

    EXPECT_CALL(adapter, WriteWithoutResponse(handle, testing::ElementsAreArray(data), testing::_))
        .WillRepeatedly(testing::SaveArg<2>(&onDone));

    EXPECT_CALL(callback, callback(services::OperationStatus::success));

    retryAdapter.WriteWithoutResponse(handle, data, [&callback](services::OperationStatus status)
        {
            callback.callback(status);
        });
    onDone(services::OperationStatus::retry);
    ExecuteAllActions();
    onDone(services::OperationStatus::retry);
    ExecuteAllActions();
    onDone(services::OperationStatus::retry);
    ExecuteAllActions();
    onDone(services::OperationStatus::success);
    ExecuteAllActions();
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_write_without_response_characteristic_with_retry_and_error)
{
    testing::StrictMock<infra::MockCallback<void(services::OperationStatus)>> callback;
    infra::Function<void(services::OperationStatus)> onDone;

    EXPECT_CALL(callback, callback(services::OperationStatus::error));
    EXPECT_CALL(adapter, WriteWithoutResponse(handle, testing::ElementsAreArray(data), testing::_))
        .WillRepeatedly(testing::SaveArg<2>(&onDone));

    retryAdapter.WriteWithoutResponse(handle, data, [&callback](services::OperationStatus status)
        {
            callback.callback(status);
        });
    onDone(services::OperationStatus::retry);
    ExecuteAllActions();
    onDone(services::OperationStatus::retry);
    ExecuteAllActions();
    onDone(services::OperationStatus::error);
    ExecuteAllActions();
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_enable_notification_characteristic)
{
    EXPECT_CALL(adapter, EnableNotification(handle, testing::Ref(onDone)));

    retryAdapter.EnableNotification(handle, onDone);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_disable_notification_characteristic)
{
    EXPECT_CALL(adapter, DisableNotification(handle, testing::Ref(onDone)));

    retryAdapter.DisableNotification(handle, onDone);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_enable_indication_characteristic)
{
    EXPECT_CALL(adapter, EnableIndication(handle, testing::Ref(onDone)));

    retryAdapter.EnableIndication(handle, onDone);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_disable_indication_characteristic)
{
    EXPECT_CALL(adapter, DisableIndication(handle, testing::Ref(onDone)));

    retryAdapter.DisableIndication(handle, onDone);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_forward_notification_received)
{
    EXPECT_CALL(stackUpdateObserver, NotificationReceived(handle, testing::ElementsAreArray(data)));
    retryAdapter.NotificationReceived(handle, data);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_forward_indication_received)
{
    infra::VerifyingFunction<void()> onDone;

    EXPECT_CALL(stackUpdateObserver, IndicationReceived(handle, testing::ElementsAreArray(data), testing::_)).WillOnce(testing::InvokeArgument<2>());
    retryAdapter.IndicationReceived(handle, data, onDone);
}
