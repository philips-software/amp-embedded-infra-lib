#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/Att.hpp"
#include "services/ble/ClaimingGattClientAdapter.hpp"
#include "services/ble/GattClient.hpp"
#include "services/ble/test_doubles/GapCentralMock.hpp"
#include "services/ble/test_doubles/GattClientMock.hpp"
#include "services/ble/test_doubles/GattMock.hpp"
#include "gmock/gmock.h"

namespace
{
    class ClaimingGattClientAdapterTest
        : public testing::Test
        , public infra::EventDispatcherFixture
    {
    public:
        testing::StrictMock<services::GattClientMock> gattClient;
        testing::StrictMock<services::AttMtuExchangeMock> attMtuExchange;
        testing::StrictMock<services::GapCentralMock> gapCentral;
        services::ClaimingGattClientAdapter adapter{ gattClient, attMtuExchange, gapCentral };
        testing::StrictMock<services::GattClientDiscoveryObserverMock> discoveryObserver{ adapter };
        testing::StrictMock<services::GattClientStackUpdateObserverMock> stackUpdateObserver{ adapter };
        testing::StrictMock<services::AttMtuExchangeObserverMock> attMtuExchangeObserver{ adapter };

        static constexpr services::AttAttribute::Handle handle = 0x1;
        static constexpr services::AttAttribute::Handle endHandle = 0x2;
    };
}

TEST_F(ClaimingGattClientAdapterTest, should_call_start_service_discovery)
{
    EXPECT_CALL(gattClient, StartServiceDiscovery());
    adapter.StartServiceDiscovery();
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_start_characteristic_discovery)
{
    EXPECT_CALL(gattClient, StartCharacteristicDiscovery(handle, endHandle));
    adapter.StartCharacteristicDiscovery(handle, endHandle);
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_start_descriptor_discovery)
{
    EXPECT_CALL(gattClient, StartDescriptorDiscovery(handle, endHandle));
    adapter.StartDescriptorDiscovery(handle, endHandle);
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_service_discovered)
{
    static const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };

    EXPECT_CALL(discoveryObserver, ServiceDiscovered(type, handle, endHandle));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.ServiceDiscovered(type, handle, endHandle);
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_characteristic_discovered)
{
    static const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };
    static const auto valueHandle = 0x2;
    static const auto properties = services::GattCharacteristic::PropertyFlags::read | services::GattCharacteristic::PropertyFlags::notify;

    EXPECT_CALL(discoveryObserver, CharacteristicDiscovered(type, handle, valueHandle, properties));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.CharacteristicDiscovered(type, handle, valueHandle, properties);
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_descriptor_discovered)
{
    static const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };

    EXPECT_CALL(discoveryObserver, DescriptorDiscovered(type, handle));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.DescriptorDiscovered(type, handle);
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_mtu_exchange)
{
    EXPECT_CALL(attMtuExchange, EffectiveAttMtuSize()).WillOnce(testing::Return(200));
    EXPECT_EQ(200, adapter.EffectiveAttMtuSize());

    infra::Function<void(services::OperationStatus)> storedOnDone;
    EXPECT_CALL(gattClient, MtuExchange(testing::_)).WillOnce(testing::SaveArg<0>(&storedOnDone));

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ services::OperationStatus::success };
    adapter.MtuExchange(onDone);
    ExecuteAllActions();
    storedOnDone(services::OperationStatus::success);

    EXPECT_CALL(attMtuExchangeObserver, ExchangedAttMtuSize());
    attMtuExchange.NotifyObservers([](auto& observer)
        {
            observer.ExchangedAttMtuSize();
        });
}

TEST_F(ClaimingGattClientAdapterTest, should_forward_notification_received)
{
    static const infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });

    EXPECT_CALL(stackUpdateObserver, NotificationReceived(handle, data));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.NotificationReceived(handle, data);
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_forward_indication_received)
{
    static const infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });

    EXPECT_CALL(stackUpdateObserver, IndicationReceived(handle, data, testing::_)).WillOnce(testing::InvokeArgument<2>());
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.IndicationReceived(handle, data, infra::VerifyingFunction<void()>());
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, can_write_without_response_while_awaiting_read)
{
    const infra::ConstByteRange readResult = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    const auto handleWrite = 0x2;
    const auto operationStatusOk = services::OperationStatus::success;

    infra::Function<void(const infra::ConstByteRange&)> onRead;
    infra::Function<void(services::OperationStatus)> onDone;
    infra::VerifyingFunction<void(const infra::ConstByteRange&)> verifyOnRead{ readResult };

    EXPECT_CALL(gattClient, Read(handle, testing::_, testing::_))
        .WillOnce(::testing::DoAll(::testing::SaveArg<1>(&onRead), ::testing::SaveArg<2>(&onDone)));
    EXPECT_CALL(gattClient, WriteWithoutResponse(handleWrite, testing::_, testing::_)).WillOnce(::testing::InvokeArgument<2>(services::OperationStatus::success));

    infra::VerifyingFunction<void(services::OperationStatus)> verifyOnDone{ operationStatusOk };
    adapter.Read(handle, verifyOnRead, verifyOnDone);

    ExecuteAllActions();

    adapter.WriteWithoutResponse(handleWrite, infra::MakeRange(std::array<uint8_t, 1>({ 42 })), infra::MockFunction<void(services::OperationStatus)>(services::OperationStatus::success));

    onRead(readResult);
    onDone(operationStatusOk);

    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_release_claimer_when_disconnected)
{
    EXPECT_CALL(gattClient, MtuExchange(testing::_));
    adapter.MtuExchange([](auto) {});
    ExecuteAllActions();

    gapCentral.ChangeState(services::GapState::standby);

    EXPECT_CALL(gattClient, MtuExchange(testing::_));
    adapter.MtuExchange([](auto) {});
    ExecuteAllActions();
}

class ClaimingGattClientAdapterStatusTest
    : public ClaimingGattClientAdapterTest
    , public testing::WithParamInterface<services::OperationStatus>
{};

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_service_discovery_complete)
{
    EXPECT_CALL(discoveryObserver, ServiceDiscoveryComplete(GetParam()));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.ServiceDiscoveryComplete(GetParam());
        });
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_characteristic_discovery_complete)
{
    EXPECT_CALL(discoveryObserver, CharacteristicDiscoveryComplete(GetParam()));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.CharacteristicDiscoveryComplete(GetParam());
        });
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_descriptor_discovery_complete)
{
    EXPECT_CALL(discoveryObserver, DescriptorDiscoveryComplete(GetParam()));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.DescriptorDiscoveryComplete(GetParam());
        });
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_read_characteristic)
{
    const infra::ConstByteRange readResult = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    infra::VerifyingFunction<void(const infra::ConstByteRange&)> onRead{ readResult };
    infra::VerifyingFunction<void(services::OperationStatus)> verifyOnDone{ GetParam() };

    EXPECT_CALL(gattClient, Read(handle, testing::_, testing::_))
        .WillOnce([&readResult](services::AttAttribute::Handle passedHandle,
                      infra::Function<void(const infra::ConstByteRange&)> onRead,
                      infra::Function<void(services::OperationStatus)> onDone)
            {
                EXPECT_EQ(passedHandle, handle);

                onRead(readResult);
                onDone(GetParam());
            });

    adapter.Read(handle, onRead, verifyOnDone);
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_write_characteristic)
{
    infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });

    EXPECT_CALL(gattClient, Write(handle, infra::ByteRangeContentsEqual(data), testing::_)).WillOnce([data](services::AttAttribute::Handle passedHandle, infra::ConstByteRange writeData, const infra::Function<void(services::OperationStatus)>& onWriteDone)
        {
            EXPECT_EQ(passedHandle, handle);
            onWriteDone(GetParam());
        });

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ GetParam() };
    adapter.Write(handle, data, onDone);
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_write_without_response_characteristic)
{
    infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });

    EXPECT_CALL(gattClient, WriteWithoutResponse(testing::_, infra::ByteRangeContentsEqual(data), testing::_))
        .WillOnce([](services::AttAttribute::Handle passedHandle,
                      infra::ConstByteRange data,
                      const infra::Function<void(services::OperationStatus)>& onDone)
            {
                EXPECT_EQ(passedHandle, handle);
                onDone(GetParam());
            });

    infra::VerifyingFunction<void(services::OperationStatus)> onWriteWithoutResponse{ GetParam() };
    adapter.WriteWithoutResponse(handle, data, onWriteWithoutResponse);
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_enable_notification_characteristic)
{
    EXPECT_CALL(gattClient, EnableNotification(handle, testing::_))
        .WillOnce([](services::AttAttribute::Handle passedHandle,
                      infra::Function<void(services::OperationStatus)> onDone)
            {
                EXPECT_EQ(passedHandle, handle);
                onDone(GetParam());
            });

    infra::VerifyingFunction<void(services::OperationStatus)> verifyOnDone{ GetParam() };
    adapter.EnableNotification(handle, verifyOnDone);
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_disable_notification_characteristic)
{
    EXPECT_CALL(gattClient, DisableNotification(handle, testing::_))
        .WillOnce([](services::AttAttribute::Handle passedHandle,
                      infra::Function<void(services::OperationStatus)> onDone)
            {
                EXPECT_EQ(passedHandle, handle);
                onDone(GetParam());
            });

    infra::VerifyingFunction<void(services::OperationStatus)> verifyOnDone{ GetParam() };
    adapter.DisableNotification(handle, verifyOnDone);
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_enable_indication_characteristic)
{
    EXPECT_CALL(gattClient, EnableIndication(handle, testing::_))
        .WillOnce([](services::AttAttribute::Handle passedHandle,
                      infra::Function<void(services::OperationStatus)> onDone)
            {
                EXPECT_EQ(passedHandle, handle);
                onDone(GetParam());
            });

    infra::VerifyingFunction<void(services::OperationStatus)> verifyOnDone{ GetParam() };
    adapter.EnableIndication(handle, verifyOnDone);
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_call_disable_indication_characteristic)
{
    EXPECT_CALL(gattClient, DisableIndication(handle, testing::_))
        .WillOnce([](services::AttAttribute::Handle passedHandle,
                      infra::Function<void(services::OperationStatus)> onDone)
            {
                EXPECT_EQ(passedHandle, handle);
                onDone(GetParam());
            });

    infra::VerifyingFunction<void(services::OperationStatus)> verifyOnDone{ GetParam() };
    adapter.DisableIndication(handle, verifyOnDone);
    ExecuteAllActions();
}

TEST_P(ClaimingGattClientAdapterStatusTest, should_block_discovery_while_characteristic_operation)
{
    EXPECT_CALL(gattClient, StartCharacteristicDiscovery(handle, endHandle));
    adapter.StartCharacteristicDiscovery(handle, endHandle);
    ExecuteAllActions();

    infra::VerifyingFunction<void(services::OperationStatus)> verifyOnDone{ GetParam() };
    adapter.DisableIndication(handle, verifyOnDone);
    ExecuteAllActions();

    EXPECT_CALL(discoveryObserver, CharacteristicDiscoveryComplete(GetParam()));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.CharacteristicDiscoveryComplete(GetParam());
        });

    EXPECT_CALL(gattClient, DisableIndication(handle, testing::_))
        .WillOnce([](services::AttAttribute::Handle passedHandle,
                      infra::Function<void(services::OperationStatus)> onDone)
            {
                EXPECT_EQ(passedHandle, handle);
                onDone(GetParam());
            });
    ExecuteAllActions();
}

INSTANTIATE_TEST_SUITE_P(ClaimingGattClientAdapterTests, ClaimingGattClientAdapterStatusTest,
    testing::Values(
        services::OperationStatus::success,
        services::OperationStatus::error));
