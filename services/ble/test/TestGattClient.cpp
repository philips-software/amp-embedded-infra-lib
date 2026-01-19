#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/stream/StringOutputStream.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/Att.hpp"
#include "services/ble/test_doubles/GattClientMock.hpp"
#include "gmock/gmock.h"

namespace
{
    services::AttAttribute::Uuid16 uuid16{ 0x42 };
    services::AttAttribute::Uuid128 uuid128{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 } };

    using GattPropertyFlags = services::GattCharacteristic::PropertyFlags;
}

TEST(GattClientTest, characteristic_implementation_supports_different_uuid_lengths)
{
    services::GattClientService service(uuid16, 0x1, 0x9);
    services::GattClientCharacteristic characteristicDefinitionA{ uuid16, 0x2, 0x3, GattPropertyFlags::none };
    services::GattClientCharacteristic characteristicDefinitionB{ uuid128, 0x2, 0x3, GattPropertyFlags::none };

    service.AddCharacteristic(characteristicDefinitionA);
    service.AddCharacteristic(characteristicDefinitionB);

    EXPECT_EQ(0x42, std::get<services::AttAttribute::Uuid16>(characteristicDefinitionA.Type()));
    EXPECT_EQ(uuid128, std::get<services::AttAttribute::Uuid128>(characteristicDefinitionB.Type()));
}

TEST(GattClientTest, characteristic_implementation_supports_different_properties)
{
    services::GattClientService service(uuid16, 0x1, 0x9);
    services::GattClientCharacteristic characteristicDefinitionA{ uuid16, 0x2, 0x3, GattPropertyFlags::write | GattPropertyFlags::indicate };
    services::GattClientCharacteristic characteristicDefinitionB{ uuid16, 0x2, 0x3, GattPropertyFlags::broadcast };

    service.AddCharacteristic(characteristicDefinitionA);
    service.AddCharacteristic(characteristicDefinitionB);

    EXPECT_EQ(GattPropertyFlags::write | GattPropertyFlags::indicate, characteristicDefinitionA.Properties());
    EXPECT_EQ(GattPropertyFlags::broadcast, characteristicDefinitionB.Properties());
}

TEST(GattClientTest, characteristic_implementation_is_added_to_service)
{
    services::GattClientService service(uuid16, 0x1, 0x9);
    services::GattClientCharacteristic characteristicDefinitionA{ uuid16, 0x2, 0x3, GattPropertyFlags::write };
    services::GattClientCharacteristic characteristicDefinitionB{ services::AttAttribute::Uuid16(0x84), 0x4, 0x5, GattPropertyFlags::none };

    service.AddCharacteristic(characteristicDefinitionA);
    service.AddCharacteristic(characteristicDefinitionB);

    EXPECT_FALSE(service.Characteristics().empty());
    EXPECT_EQ(0x84, std::get<services::AttAttribute::Uuid16>(service.Characteristics().front().Type()));
}

TEST(GattClientTest, OperationStatusSuccess)
{
    infra::StringOutputStream::WithStorage<32> stream;
    services::OperationStatus status = services::OperationStatus::success;

    stream << status;

    EXPECT_EQ("success", stream.Storage());
}

TEST(GattClientTest, OperationStatusRetry)
{
    infra::StringOutputStream::WithStorage<32> stream;
    services::OperationStatus status = services::OperationStatus::retry;

    stream << status;

    EXPECT_EQ("retry", stream.Storage());
}

TEST(GattClientTest, OperationStatusError)
{
    infra::StringOutputStream::WithStorage<32> stream;
    services::OperationStatus status = services::OperationStatus::error;

    stream << status;

    EXPECT_EQ("error", stream.Storage());
}

class GattClientCharacteristicTest
    : public testing::Test
    , public infra::EventDispatcherFixture
{
public:
    GattClientCharacteristicTest()
        : service(uuid16, 0x1, 0x9)
        , characteristic(operations, uuid16, characteristicHandle, characteristicValueHandle, GattPropertyFlags::write)
    {
        gattUpdateObserver.Attach(characteristic);
    }

    static constexpr services::AttAttribute::Handle characteristicHandle = 0x2;
    static constexpr services::AttAttribute::Handle characteristicValueHandle = 0x3;
    static constexpr auto result = services::OperationStatus::success;

    testing::StrictMock<services::GattClientCharacteristicOperationsMock> operations;
    services::GattClientService service;
    services::GattClientCharacteristic characteristic;
    testing::StrictMock<services::GattClientCharacteristicUpdateObserverMock> gattUpdateObserver;

    testing::StrictMock<services::GattClientDiscoveryMock> discovery;
};

TEST_F(GattClientCharacteristicTest, receives_valid_notification_should_notify_observers)
{
    const auto data = infra::MakeStringByteRange("string");

    EXPECT_CALL(gattUpdateObserver, NotificationReceived(infra::ByteRangeContentsEqual(data)));
    operations.infra::Subject<services::GattClientStackUpdateObserver>::NotifyObservers([&data](auto& observer)
        {
            observer.NotificationReceived(characteristicValueHandle, data);
        });
}

TEST_F(GattClientCharacteristicTest, receives_valid_indication_should_notify_observers)
{
    EXPECT_CALL(gattUpdateObserver, IndicationReceived(infra::ByteRangeContentsEqual(infra::MakeStringByteRange("string")), testing::_))
        .WillOnce(testing::InvokeArgument<1>());

    operations.infra::Subject<services::GattClientStackUpdateObserver>::NotifyObservers([](auto& observer)
        {
            observer.IndicationReceived(characteristicValueHandle, infra::MakeStringByteRange("string"), infra::MockFunction<void()>());
        });
}

TEST_F(GattClientCharacteristicTest, receives_invalid_notification_should_not_notify_observers)
{
    const services::AttAttribute::Handle invalidCharacteristicValueHandle = 0x7;

    operations.infra::Subject<services::GattClientStackUpdateObserver>::NotifyObservers([&invalidCharacteristicValueHandle](auto& observer)
        {
            observer.NotificationReceived(invalidCharacteristicValueHandle, infra::MakeStringByteRange("string"));
        });
}

TEST_F(GattClientCharacteristicTest, receives_invalid_indication_should_not_notify_observers)
{
    const services::AttAttribute::Handle invalidCharacteristicValueHandle = 0x7;

    operations.infra::Subject<services::GattClientStackUpdateObserver>::NotifyObservers([&invalidCharacteristicValueHandle](auto& observer)
        {
            observer.IndicationReceived(invalidCharacteristicValueHandle, infra::MakeStringByteRange("string"), infra::MockFunction<void()>());
        });
}

TEST_F(GattClientCharacteristicTest, should_read_characteristic_and_callback_with_data_received)
{
    const auto data = infra::MakeStringByteRange("string");
    infra::VerifyingFunction<void(const infra::ConstByteRange&)> onResponse{ data };

    EXPECT_CALL(operations, Read(characteristicValueHandle, ::testing::_, testing::_))
        .WillOnce([&data](services::AttAttribute::Handle, infra::Function<void(const infra::ConstByteRange&)> onResponse, infra::Function<void(services::OperationStatus)> onDone)
            {
                onResponse(data);
                onDone(result);
            });

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ result };
    characteristic.Read(onResponse, onDone);
}

TEST_F(GattClientCharacteristicTest, should_write_characteristic_and_callback)
{
    const auto data = infra::MakeStringByteRange("string");

    EXPECT_CALL(operations, Write(characteristicValueHandle, infra::ByteRangeContentsEqual(data), testing::_)).WillOnce([&data](services::AttAttribute::Handle, infra::ConstByteRange, infra::Function<void(services::OperationStatus)> onDone)
        {
            onDone(result);
        });

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ result };
    characteristic.Write(data, onDone);
}

TEST_F(GattClientCharacteristicTest, should_write_without_response_characteristic)
{
    const auto data = infra::MakeStringByteRange("string");

    EXPECT_CALL(operations, WriteWithoutResponse(characteristicValueHandle, infra::ByteRangeContentsEqual(data), testing::_)).WillOnce([](services::AttAttribute::Handle handle, infra::ConstByteRange, infra::Function<void(services::OperationStatus)> onDone)
        {
            onDone(result);
        });

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ result };
    characteristic.WriteWithoutResponse(data, onDone);
}

TEST_F(GattClientCharacteristicTest, should_enable_notification_characteristic_and_callback)
{
    EXPECT_CALL(operations, EnableNotification(characteristicValueHandle, ::testing::_)).WillOnce([](services::AttAttribute::Handle, infra::Function<void(services::OperationStatus)> onDone)
        {
            onDone(result);
        });

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ result };
    characteristic.EnableNotification(onDone);
}

TEST_F(GattClientCharacteristicTest, should_disable_notification_characteristic_and_callback)
{
    EXPECT_CALL(operations, DisableNotification(characteristicValueHandle, ::testing::_)).WillOnce([](services::AttAttribute::Handle, infra::Function<void(services::OperationStatus)> onDone)
        {
            onDone(result);
        });

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ result };
    characteristic.DisableNotification(onDone);
}

TEST_F(GattClientCharacteristicTest, should_enable_indication_characteristic_and_callback)
{
    EXPECT_CALL(operations, EnableIndication(characteristicValueHandle, ::testing::_)).WillOnce([](services::AttAttribute::Handle, infra::Function<void(services::OperationStatus)> onDone)
        {
            onDone(result);
        });

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ result };
    characteristic.EnableIndication(onDone);
}

TEST_F(GattClientCharacteristicTest, should_disable_indication_characteristic_and_callback)
{
    EXPECT_CALL(operations, DisableIndication(characteristicValueHandle, ::testing::_)).WillOnce([](services::AttAttribute::Handle, infra::Function<void(services::OperationStatus)> onDone)
        {
            onDone(result);
        });

    infra::VerifyingFunction<void(services::OperationStatus)> onDone{ result };
    characteristic.DisableIndication(onDone);
}
