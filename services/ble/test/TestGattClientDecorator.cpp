
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/GattClient.hpp"
#include "services/ble/GattClientDecorator.hpp"
#include "services/ble/test_doubles/GattClientMock.hpp"
#include "gmock/gmock.h"
#include <array>
#include <functional>
#include <gtest/gtest.h>
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"

namespace
{
    class GattClientDecoratorTest
        : public testing::Test
    {
    public:
        services::GattClientDiscoveryMock discovery;
        services::GattClientCharacteristicOperationsMock operations;
        services::GattClientDecorator gattClientDecorator{ discovery, operations };
        services::GattClientCharacteristicOperationsObserverMock operationsObserver{ gattClientDecorator };
        services::GattClientDiscoveryObserverMock discoveryObserver{ gattClientDecorator };
    };
}

// // Implementation of GattClientDiscovery
// void StartServiceDiscovery() override;
// void StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
// void StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;

// // Implementation of GattClientDiscoveryObserver
// void ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
// void ServiceDiscoveryComplete() override;
// void CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties) override;
// void CharacteristicDiscoveryComplete() override;
// void DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle) override;
// void DescriptorDiscoveryComplete() override;

// // Implementation of GattClientCharacteristicOperations
// void Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone) override;
// void Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone) override;
// void WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data) override;
// void EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
// void DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
// void EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
// void DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;

// // Implementation of GattClientCharacteristicOperationsObserver
// AttAttribute::Handle CharacteristicValueHandle() const override;

TEST_F(GattClientDecoratorTest, should_start_service_discovery)
{
    EXPECT_CALL(discovery, StartServiceDiscovery());

    gattClientDecorator.StartServiceDiscovery();
}

TEST_F(GattClientDecoratorTest, should_start_characteristic_discovery)
{
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    EXPECT_CALL(discovery, StartCharacteristicDiscovery(handle, endHandle));
    gattClientDecorator.StartCharacteristicDiscovery(handle, endHandle);
}

TEST_F(GattClientDecoratorTest, should_start_descriptor_discovery)
{
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    EXPECT_CALL(discovery, StartDescriptorDiscovery(handle, endHandle));
    gattClientDecorator.StartDescriptorDiscovery(handle, endHandle);
}

TEST_F(GattClientDecoratorTest, should_discover_service)
{
    const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    std::cout << &type << std::endl;

    EXPECT_CALL(discoveryObserver, ServiceDiscovered(testing::Ref(type), handle, endHandle));
    gattClientDecorator.ServiceDiscovered(type, handle, endHandle);
}

TEST_F(GattClientDecoratorTest, should_complete_service_discovery)
{
    EXPECT_CALL(discoveryObserver, ServiceDiscoveryComplete());
    gattClientDecorator.ServiceDiscoveryComplete();
}

TEST_F(GattClientDecoratorTest, should_discover_characteristic)
{
    const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };
    const auto handle = 0x1;
    const auto valueHandle = 0x2;
    const auto properties = services::GattCharacteristic::PropertyFlags::read | services::GattCharacteristic::PropertyFlags::notify;

    EXPECT_CALL(discoveryObserver, CharacteristicDiscovered(testing::Ref(type), handle, valueHandle, properties));
    gattClientDecorator.CharacteristicDiscovered(type, handle, valueHandle, properties);
}

TEST_F(GattClientDecoratorTest, should_complete_characteristic_discovery)
{
    EXPECT_CALL(discoveryObserver, CharacteristicDiscoveryComplete());
    gattClientDecorator.CharacteristicDiscoveryComplete();
}

TEST_F(GattClientDecoratorTest, should_discover_descriptor)
{
    const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };
    const auto handle = 0x1;

    EXPECT_CALL(discoveryObserver, DescriptorDiscovered(testing::Ref(type), handle));
    gattClientDecorator.DescriptorDiscovered(type, handle);
}

TEST_F(GattClientDecoratorTest, should_complete_descriptor_discovery)
{
    EXPECT_CALL(discoveryObserver, DescriptorDiscoveryComplete());
    gattClientDecorator.DescriptorDiscoveryComplete();
}

TEST_F(GattClientDecoratorTest, should_read_characteristic)
{
    const infra::ConstByteRange readResult = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    const auto result = 123;

    EXPECT_CALL(operations, Read(testing::Ref(operationsObserver), testing::_, testing::_)).WillOnce([&readResult, result](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(const infra::ConstByteRange&)> onRead, infra::Function<void(uint8_t)> onDone)
        {
            onRead(readResult);
            onDone(result);
        });
    gattClientDecorator.Read(operationsObserver, infra::MockFunction<void(const infra::ConstByteRange&)>(readResult), infra::MockFunction<void(uint8_t)>(result));
}

TEST_F(GattClientDecoratorTest, should_write_characteristic)
{
    infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    const auto result = 123;

    EXPECT_CALL(operations, Write(testing::Ref(operationsObserver), infra::ByteRangeContentsEqual(data), testing::_)).WillOnce([result](const services::GattClientCharacteristicOperationsObserver&, infra::ConstByteRange, infra::Function<void(uint8_t)> onDone)
        {
            onDone(result);
        });
    gattClientDecorator.Write(operationsObserver, data, infra::MockFunction<void(uint8_t)>(result));
}

TEST_F(GattClientDecoratorTest, should_write_without_response_characteristic)
{
    infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });

    EXPECT_CALL(operations, WriteWithoutResponse(testing::Ref(operationsObserver), infra::ByteRangeContentsEqual(data)));
    gattClientDecorator.WriteWithoutResponse(operationsObserver, data);
}

TEST_F(GattClientDecoratorTest, should_enable_notification_characteristic)
{
    const auto result = 123;

    EXPECT_CALL(operations, EnableNotification(testing::Ref(operationsObserver), testing::_)).WillOnce([result](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(uint8_t)> onDone)
        {
            onDone(result);
        });
    gattClientDecorator.EnableNotification(operationsObserver, infra::MockFunction<void(uint8_t)>(result));
}

TEST_F(GattClientDecoratorTest, should_disable_notification_characteristic)
{
    const auto result = 123;

    EXPECT_CALL(operations, DisableNotification(testing::Ref(operationsObserver), testing::_)).WillOnce([result](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(uint8_t)> onDone)
        {
            onDone(result);
        });
    gattClientDecorator.DisableNotification(operationsObserver, infra::MockFunction<void(uint8_t)>(result));
}

TEST_F(GattClientDecoratorTest, should_enable_indication_characteristic)
{
    const auto result = 123;

    EXPECT_CALL(operations, EnableIndication(testing::Ref(operationsObserver), testing::_)).WillOnce([result](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(uint8_t)> onDone)
        {
            onDone(result);
        });
    gattClientDecorator.EnableIndication(operationsObserver, infra::MockFunction<void(uint8_t)>(result));
}   

TEST_F(GattClientDecoratorTest, should_disable_indication_characteristic)
{
    const auto result = 123;

    EXPECT_CALL(operations, DisableIndication(testing::Ref(operationsObserver), testing::_)).WillOnce([result](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(uint8_t)> onDone)
        {
            onDone(result);
        });
    gattClientDecorator.DisableIndication(operationsObserver, infra::MockFunction<void(uint8_t)>(result));
}

