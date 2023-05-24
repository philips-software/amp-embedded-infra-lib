#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
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

    EXPECT_EQ(0x42, characteristicDefinitionA.Type().Get<services::AttAttribute::Uuid16>());
    EXPECT_EQ((infra::BigEndian<std::array<uint8_t, 16>>{ { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 } }),
        characteristicDefinitionB.Type().Get<services::AttAttribute::Uuid128>());
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
    services::GattClientCharacteristic characteristicDefinitionA{ services::AttAttribute::Uuid16(0x42), 0x2, 0x3, GattPropertyFlags::write };
    services::GattClientCharacteristic characteristicDefinitionB{ services::AttAttribute::Uuid16(0x84), 0x4, 0x5, GattPropertyFlags::none };

    service.AddCharacteristic(characteristicDefinitionA);
    service.AddCharacteristic(characteristicDefinitionB);

    EXPECT_FALSE(service.Characteristics().empty());
    EXPECT_EQ(0x84, service.Characteristics().front().Type().Get<services::AttAttribute::Uuid16>());
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

    static const services::AttAttribute::Handle characteristicHandle = 0x2;
    static const services::AttAttribute::Handle characteristicValueHandle = 0x3;

    testing::StrictMock<services::GattClientCharacteristicOperationsMock> operations;
    services::GattClientService service;
    services::GattClientCharacteristic characteristic;
    testing::StrictMock<services::GattClientCharacteristicUpdateObserverMock> gattUpdateObserver;
};

TEST_F(GattClientCharacteristicTest, receives_valid_notification_should_notify_observers)
{
    EXPECT_CALL(gattUpdateObserver, UpdateReceived(infra::ByteRangeContentsEqual(infra::MakeStringByteRange("string"))));

    operations.infra::Subject<services::GattClientStackUpdateObserver>::NotifyObservers([](auto& observer)
        { observer.UpdateReceived(characteristicValueHandle, infra::MakeStringByteRange("string")); });
}

TEST_F(GattClientCharacteristicTest, receives_invalid_notification_should_not_notify_observers)
{
    const services::AttAttribute::Handle invalidCharacteristicValueHandle = 0x7;

    operations.infra::Subject<services::GattClientStackUpdateObserver>::NotifyObservers([&invalidCharacteristicValueHandle](auto& observer)
        { observer.UpdateReceived(invalidCharacteristicValueHandle, infra::MakeStringByteRange("string")); });
}

TEST_F(GattClientCharacteristicTest, should_read_characteristic_and_callback_with_data_received)
{
    EXPECT_CALL(operations, Read(testing::Ref(characteristic), ::testing::_)).WillOnce([](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(const infra::ConstByteRange&)> onResponse)
        { onResponse(infra::MakeStringByteRange("string")); });
    characteristic.Read([](const infra::ConstByteRange& response)
        { EXPECT_TRUE(response == infra::MakeStringByteRange("string")); });
}

TEST_F(GattClientCharacteristicTest, should_write_characteristic_and_callback)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);

    EXPECT_CALL(operations, Write(testing::Ref(characteristic), infra::ByteRangeContentsEqual(infra::MakeStringByteRange("string")), ::testing::_)).WillOnce([](const services::GattClientCharacteristicOperationsObserver&, infra::ConstByteRange, infra::Function<void()> onDone)
        { onDone(); });
    characteristic.Write(infra::MakeStringByteRange("string"), [&callback]()
        { callback.callback(); });
}

TEST_F(GattClientCharacteristicTest, should_write_without_response_characteristic)
{
    EXPECT_CALL(operations, WriteWithoutResponse(testing::Ref(characteristic), infra::ByteRangeContentsEqual(infra::MakeStringByteRange("string"))));
    characteristic.WriteWithoutResponse(infra::MakeStringByteRange("string"));
}

TEST_F(GattClientCharacteristicTest, should_enable_notification_characteristic_and_callback)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);

    EXPECT_CALL(operations, EnableNotification(testing::Ref(characteristic), ::testing::_)).WillOnce([](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void()> onDone)
        { onDone(); });
    characteristic.EnableNotification([&callback]()
        { callback.callback(); });
}

TEST_F(GattClientCharacteristicTest, should_disable_notification_characteristic_and_callback)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);

    EXPECT_CALL(operations, DisableNotification(testing::Ref(characteristic), ::testing::_)).WillOnce([](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void()> onDone)
        { onDone(); });
    characteristic.DisableNotification([&callback]()
        { callback.callback(); });
}

TEST_F(GattClientCharacteristicTest, should_enable_indication_characteristic_and_callback)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);

    EXPECT_CALL(operations, EnableIndication(testing::Ref(characteristic), ::testing::_)).WillOnce([](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void()> onDone)
        { onDone(); });
    characteristic.EnableIndication([&callback]()
        { callback.callback(); });
}

TEST_F(GattClientCharacteristicTest, should_disable_indication_characteristic_and_callback)
{
    infra::MockCallback<void()> callback;
    EXPECT_CALL(callback, callback);

    EXPECT_CALL(operations, DisableIndication(testing::Ref(characteristic), ::testing::_)).WillOnce([](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void()> onDone)
        { onDone(); });
    characteristic.DisableIndication([&callback]()
        { callback.callback(); });
}

namespace
{
    class GattClientDiscoveryDecoratorTest
        : public testing::Test
    {
    public:
        services::GattClientDiscoveryMock gattDiscovery;
        services::GattClientDiscoveryDecorator decorator{ gattDiscovery };
        services::GattClientDiscoveryObserverMock gattDiscoveryObserver{ decorator };
    };
}

TEST_F(GattClientDiscoveryDecoratorTest, forward_service_discovered_event_to_observers)
{
    EXPECT_CALL(gattDiscoveryObserver, ServiceDiscovered(services::AttAttribute::Uuid(uuid128), 0x24, 0xAD));
    EXPECT_CALL(gattDiscoveryObserver, ServiceDiscovered(services::AttAttribute::Uuid(uuid16), 0xDE, 0x42));
    EXPECT_CALL(gattDiscoveryObserver, ServiceDiscoveryComplete());

    gattDiscovery.NotifyObservers([](auto& obs)
        {
        obs.ServiceDiscovered(services::AttAttribute::Uuid(uuid128), 0x24, 0xAD);
        obs.ServiceDiscovered(services::AttAttribute::Uuid(uuid16), 0xDE, 0x42);
        obs.ServiceDiscoveryComplete(); });
}

TEST_F(GattClientDiscoveryDecoratorTest, forward_characteristics_discovered_event_to_observers)
{
    EXPECT_CALL(gattDiscoveryObserver, CharacteristicDiscovered(services::AttAttribute::Uuid(uuid128), 0x24, 0xAD, GattPropertyFlags::notify));
    EXPECT_CALL(gattDiscoveryObserver, CharacteristicDiscovered(services::AttAttribute::Uuid(uuid16), 0xDE, 0x42, GattPropertyFlags::none));
    EXPECT_CALL(gattDiscoveryObserver, CharacteristicDiscoveryComplete());

    gattDiscovery.NotifyObservers([](auto& obs)
        {
        obs.CharacteristicDiscovered(services::AttAttribute::Uuid(uuid128), 0x24, 0xAD, GattPropertyFlags::notify);
        obs.CharacteristicDiscovered(services::AttAttribute::Uuid(uuid16), 0xDE, 0x42, GattPropertyFlags::none);
        obs.CharacteristicDiscoveryComplete(); });
}

TEST_F(GattClientDiscoveryDecoratorTest, forward_descriptors_discovered_event_to_observers)
{
    EXPECT_CALL(gattDiscoveryObserver, DescriptorDiscovered(services::AttAttribute::Uuid(uuid128), 0x24));
    EXPECT_CALL(gattDiscoveryObserver, DescriptorDiscovered(services::AttAttribute::Uuid(uuid16), 0xDE));
    EXPECT_CALL(gattDiscoveryObserver, DescriptorDiscoveryComplete());

    gattDiscovery.NotifyObservers([](auto& obs)
        {
        obs.DescriptorDiscovered(services::AttAttribute::Uuid(uuid128), 0x24);
        obs.DescriptorDiscovered(services::AttAttribute::Uuid(uuid16), 0xDE);
        obs.DescriptorDiscoveryComplete(); });
}

TEST_F(GattClientDiscoveryDecoratorTest, forward_all_calls_to_subject)
{
    services::GattService service{ uuid16, 0x1, 0x9 };

    EXPECT_CALL(gattDiscovery, StartServiceDiscovery());
    decorator.StartServiceDiscovery();

    EXPECT_CALL(gattDiscovery, StartCharacteristicDiscovery(::testing::Ref(service)));
    decorator.StartCharacteristicDiscovery(service);

    EXPECT_CALL(gattDiscovery, StartDescriptorDiscovery(::testing::Ref(service)));
    decorator.StartDescriptorDiscovery(service);
}