#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/ClaimingGattClientAdapter.hpp"
#include "services/ble/GattClient.hpp"
#include "services/ble/test_doubles/GattClientMock.hpp"
#include "services/ble/test_doubles/GattMock.hpp"
#include "gmock/gmock.h"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>

namespace
{
    class ClaimingGattClientAdapterTest
        : public testing::Test
        , public infra::EventDispatcherFixture
    {
    public:
        testing::StrictMock<services::GattClientMock> gattClient;
        testing::StrictMock<services::AttMtuExchangeMock> attMtuExchange;
        services::ClaimingGattClientAdapter adapter{ gattClient, attMtuExchange };
        testing::StrictMock<services::GattClientCharacteristicOperationsObserverMock> characteristicsOperationsObserver{ adapter };
        testing::StrictMock<services::GattClientDiscoveryObserverMock> discoveryObserver{ adapter };
        testing::StrictMock<services::GattClientStackUpdateObserverMock> stackUpdateObserver{ adapter };
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
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    EXPECT_CALL(gattClient, StartCharacteristicDiscovery(handle, endHandle));
    adapter.StartCharacteristicDiscovery(handle, endHandle);
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_start_descriptor_discovery)
{
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    EXPECT_CALL(gattClient, StartDescriptorDiscovery(handle, endHandle));
    adapter.StartDescriptorDiscovery(handle, endHandle);
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_service_discovered)
{
    static const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };
    static const auto handle = 0x1;
    static const auto endHandle = 0x2;

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
    static const auto handle = 0x1;
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
    static const auto handle = 0x1;

    EXPECT_CALL(discoveryObserver, DescriptorDiscovered(type, handle));
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.DescriptorDiscovered(type, handle);
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_service_discovery_complete)
{
    EXPECT_CALL(discoveryObserver, ServiceDiscoveryComplete());
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.ServiceDiscoveryComplete();
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_characteristic_discovery_complete)
{
    EXPECT_CALL(discoveryObserver, CharacteristicDiscoveryComplete());
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.CharacteristicDiscoveryComplete();
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_descriptor_discovery_complete)
{
    EXPECT_CALL(discoveryObserver, DescriptorDiscoveryComplete());
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.DescriptorDiscoveryComplete();
        });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_read_characteristic)
{
    const infra::ConstByteRange readResult = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    const auto result = 123;
    const auto handle = 0x1;

    EXPECT_CALL(gattClient, Read(testing::_, testing::_, testing::_))
        .WillOnce([&readResult, handle, result](const services::GattClientObserver& observer,
                      infra::Function<void(const infra::ConstByteRange&)> onRead,
                      infra::Function<void(uint8_t)> onDone)
            {
                EXPECT_EQ(observer.CharacteristicValueHandle(), handle);

                onRead(readResult);
                onDone(result);
            });

    EXPECT_CALL(characteristicsOperationsObserver, CharacteristicValueHandle).WillOnce(testing::Return(handle));
    adapter.Read(characteristicsOperationsObserver,
        infra::MockFunction<void(const infra::ConstByteRange&)>(readResult),
        infra::MockFunction<void(uint8_t)>(result));
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_write_characteristic)
{
    infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    const auto result = 123;
    const auto handle = 0x1;

    EXPECT_CALL(gattClient, Write(testing::_, infra::ByteRangeContentsEqual(data), testing::_))
        .WillOnce([handle](const services::GattClientObserver& observer,
                      infra::ConstByteRange data,
                      infra::Function<void(uint8_t)> onDone)
            {
                EXPECT_EQ(observer.CharacteristicValueHandle(), handle);
                onDone(123);
            });

    EXPECT_CALL(characteristicsOperationsObserver, CharacteristicValueHandle).WillOnce(testing::Return(handle));
    adapter.Write(characteristicsOperationsObserver, data, infra::MockFunction<void(uint8_t)>(result));
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_write_without_response_characteristic)
{
    infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    const auto handle = 0x1;

    EXPECT_CALL(gattClient, WriteWithoutResponse(testing::_, infra::ByteRangeContentsEqual(data)))
        .WillOnce([handle](const services::GattClientObserver& observer,
                      infra::ConstByteRange data)
            {
                EXPECT_EQ(observer.CharacteristicValueHandle(), handle);
            });

    EXPECT_CALL(characteristicsOperationsObserver, CharacteristicValueHandle).WillOnce(testing::Return(handle));
    adapter.WriteWithoutResponse(characteristicsOperationsObserver, data);
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_enable_notification_characteristic)
{
    const auto result = 123;
    const auto handle = 0x1;

    EXPECT_CALL(gattClient, EnableNotification(testing::_, testing::_))
        .WillOnce([handle, result](const services::GattClientObserver& observer,
                      infra::Function<void(uint8_t)> onDone)
            {
                EXPECT_EQ(observer.CharacteristicValueHandle(), handle);
                onDone(result);
            });

    EXPECT_CALL(characteristicsOperationsObserver, CharacteristicValueHandle).WillOnce(testing::Return(handle));
    adapter.EnableNotification(characteristicsOperationsObserver, infra::MockFunction<void(uint8_t)>(result));
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_disable_notification_characteristic)
{
    const auto result = 123;
    const auto handle = 0x1;

    EXPECT_CALL(gattClient, DisableNotification(testing::_, testing::_))
        .WillOnce([handle, result](const services::GattClientObserver& observer,
                      infra::Function<void(uint8_t)> onDone)
            {
                EXPECT_EQ(observer.CharacteristicValueHandle(), handle);
                onDone(result);
            });

    EXPECT_CALL(characteristicsOperationsObserver, CharacteristicValueHandle).WillOnce(testing::Return(handle));
    adapter.DisableNotification(characteristicsOperationsObserver, infra::MockFunction<void(uint8_t)>(result));
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_enable_indication_characteristic)
{
    const auto result = 123;
    const auto handle = 0x1;

    EXPECT_CALL(gattClient, EnableIndication(testing::_, testing::_))
        .WillOnce([handle, result](const services::GattClientObserver& observer,
                      infra::Function<void(uint8_t)> onDone)
            {
                EXPECT_EQ(observer.CharacteristicValueHandle(), handle);
                onDone(result);
            });

    EXPECT_CALL(characteristicsOperationsObserver, CharacteristicValueHandle).WillOnce(testing::Return(handle));
    adapter.EnableIndication(characteristicsOperationsObserver, infra::MockFunction<void(uint8_t)>(result));
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_call_disable_indication_characteristic)
{
    const auto result = 123;
    const auto handle = 0x1;

    EXPECT_CALL(gattClient, DisableIndication(testing::_, testing::_))
        .WillOnce([handle, result](const services::GattClientObserver& observer,
                      infra::Function<void(uint8_t)> onDone)
            {
                EXPECT_EQ(observer.CharacteristicValueHandle(), handle);
                onDone(result);
            });

    EXPECT_CALL(characteristicsOperationsObserver, CharacteristicValueHandle).WillOnce(testing::Return(handle));
    adapter.DisableIndication(characteristicsOperationsObserver, infra::MockFunction<void(uint8_t)>(result));
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_block_discovery_while_characteristic_operation)
{
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    EXPECT_CALL(gattClient, StartCharacteristicDiscovery(handle, endHandle));
    adapter.StartCharacteristicDiscovery(handle, endHandle);
    ExecuteAllActions();

    const auto result = 123;

    EXPECT_CALL(characteristicsOperationsObserver, CharacteristicValueHandle).WillOnce(testing::Return(handle));
    adapter.DisableIndication(characteristicsOperationsObserver, infra::MockFunction<void(uint8_t)>(result));
    ExecuteAllActions();

    EXPECT_CALL(discoveryObserver, CharacteristicDiscoveryComplete());
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.CharacteristicDiscoveryComplete();
        });

    EXPECT_CALL(gattClient, DisableIndication(testing::_, testing::_))
        .WillOnce([handle, result](const services::GattClientObserver& observer,
                      infra::Function<void(uint8_t)> onDone)
            {
                EXPECT_EQ(observer.CharacteristicValueHandle(), handle);
                onDone(result);
            });
    ExecuteAllActions();
}

TEST_F(ClaimingGattClientAdapterTest, should_forward_notification_received)
{
    static const auto handle = 0x1;
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
    static const auto handle = 0x1;
    static const infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });

    EXPECT_CALL(stackUpdateObserver, IndicationReceived(handle, data, testing::_)).WillOnce(testing::InvokeArgument<2>());
    gattClient.NotifyObservers([](auto& observer)
        {
            observer.IndicationReceived(handle, data, infra::MockFunction<void()>());
        });
    ExecuteAllActions();
}
