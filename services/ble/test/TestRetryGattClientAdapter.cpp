#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/GattClient.hpp"
#include "services/ble/RetryGattClientCharacteristicsOperations.hpp"
#include "services/ble/test_doubles/ClaimingGattClientAdapterMock.hpp"
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
        testing::StrictMock<services::ClaimingGattClientAdapterMock> adapter{ gattClient, attMtuExchange };
        testing::StrictMock<services::RetryGattClientCharacteristicsOperations> retryAdapter{ adapter };
        testing::StrictMock<services::GattClientStackUpdateObserverMock> stackUpdateObserver{ retryAdapter };

        const uint16_t handle = 0x1;
        std::array<uint8_t, 4> data{ 0x01, 0x02, 0x03, 0x04 };
        infra::Function<void(services::OperationStatus)> onWriteDoneMock;
        infra::Function<void(uint8_t)> onDoneMock;
    };
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_read_characteristic)
{
    infra::Function<void(const infra::ConstByteRange&)> onReadMock;

    EXPECT_CALL(adapter, Read(handle, testing::Ref(onReadMock), testing::Ref(onDoneMock)));

    retryAdapter.Read(handle,
        onReadMock,
        onDoneMock);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_write_characteristic)
{
    EXPECT_CALL(adapter, Write(handle, testing::ElementsAreArray(data), testing::Ref(onDoneMock)));

    retryAdapter.Write(handle,
        data,
        onDoneMock);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_write_without_response_characteristic_and_error)
{
    infra::VerifyingFunction<void(services::OperationStatus)> callback(services::OperationStatus::error);

    EXPECT_CALL(adapter, WriteWithoutResponse(handle, testing::ElementsAreArray(data), testing::_))
        .WillOnce(testing::SaveArg<2>(&onWriteDoneMock));

    retryAdapter.WriteWithoutResponse(handle, data, callback);
    onWriteDoneMock(services::OperationStatus::error);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_write_without_response_characteristic_with_retry)
{
    infra::VerifyingFunction<void(services::OperationStatus)> callback(services::OperationStatus::success);

    EXPECT_CALL(adapter, WriteWithoutResponse(handle, testing::ElementsAreArray(data), testing::_))
        .WillRepeatedly(testing::SaveArg<2>(&onWriteDoneMock));

    retryAdapter.WriteWithoutResponse(handle, data, callback);
    onWriteDoneMock(services::OperationStatus::retry);
    onWriteDoneMock(services::OperationStatus::retry);
    onWriteDoneMock(services::OperationStatus::retry);
    onWriteDoneMock(services::OperationStatus::success);
    ExecuteAllActions();
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_write_without_response_characteristic_with_retry_and_error)
{
    infra::VerifyingFunction<void(services::OperationStatus)> callback(services::OperationStatus::error);

    EXPECT_CALL(adapter, WriteWithoutResponse(handle, testing::ElementsAreArray(data), testing::_))
        .WillRepeatedly(testing::SaveArg<2>(&onWriteDoneMock));

    retryAdapter.WriteWithoutResponse(handle, data, callback);
    onWriteDoneMock(services::OperationStatus::retry);
    onWriteDoneMock(services::OperationStatus::retry);
    onWriteDoneMock(services::OperationStatus::error);
    ExecuteAllActions();
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_enable_notification_characteristic)
{
    EXPECT_CALL(adapter, EnableNotification(handle, testing::Ref(onDoneMock)));

    retryAdapter.EnableNotification(handle, onDoneMock);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_disable_notification_characteristic)
{
    EXPECT_CALL(adapter, DisableNotification(handle, testing::Ref(onDoneMock)));

    retryAdapter.DisableNotification(handle, onDoneMock);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_enable_indication_characteristic)
{
    EXPECT_CALL(adapter, EnableIndication(handle, testing::Ref(onDoneMock)));

    retryAdapter.EnableIndication(handle, onDoneMock);
}

TEST_F(RetryGattClientCharacteristicsOperationsTest, should_call_disable_indication_characteristic)
{
    EXPECT_CALL(adapter, DisableIndication(handle, testing::Ref(onDoneMock)));

    retryAdapter.DisableIndication(handle, onDoneMock);
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
