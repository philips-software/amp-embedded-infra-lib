
#include "infra/event/ClaimableResource.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/event/test_helper/EventDispatcherFixture.hpp"
#include "infra/util/Function.hpp"
#include "infra/util/test_helper/MemoryRangeMatcher.hpp"
#include "infra/util/test_helper/MockCallback.hpp"
#include "services/ble/GattClient.hpp"
#include "services/ble/GattClientDecorator.hpp"
#include "services/ble/test_doubles/GattClientMock.hpp"
#include "gmock/gmock.h"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>

namespace
{
    class GattClientDecoratorTest
        : public testing::Test
        , public infra::EventDispatcherFixture
    {
    public:
        testing::StrictMock<services::GattClientCharacteristicOperationsMock> operations;
        infra::ClaimableResource resource;
        infra::ClaimableResource::Claimer claimer{ resource };
        infra::MockCallback<void()> mockCallback;
        services::GattClientCharacteristicOperationsDecorator decorator{ operations, resource };
        testing::StrictMock<services::GattClientCharacteristicOperationsObserverMock> operationsObserver{ decorator };
    };
}

TEST_F(GattClientDecoratorTest, should_claim_read_characteristic_and_release_when_done)
{
    const infra::ConstByteRange readResult = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    const auto result = 123;

    claimer.Claim([]() {});
    decorator.Read(operationsObserver, infra::MockFunction<void(const infra::ConstByteRange&)>(readResult), infra::MockFunction<void(uint8_t)>(result));

    infra::Function<void(uint8_t)> readComplete;
    EXPECT_CALL(operations, Read(testing::Ref(operationsObserver), testing::_, testing::_)).WillOnce([&readResult, &readComplete](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(const infra::ConstByteRange&)> onRead, infra::Function<void(uint8_t)> onDone)
        {
            onRead(readResult);
            readComplete = onDone;
        });
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    readComplete(result);
    ExecuteAllActions();
}

TEST_F(GattClientDecoratorTest, should_claim_write_characteristic_and_release_when_done)
{
    infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });
    const auto result = 123;

    claimer.Claim([]() {});
    decorator.Write(operationsObserver, data, infra::MockFunction<void(uint8_t)>(result));

    infra::Function<void(uint8_t)> writeComplete;
    EXPECT_CALL(operations, Write(testing::Ref(operationsObserver), infra::ByteRangeContentsEqual(data), testing::_)).WillOnce([&writeComplete](const services::GattClientCharacteristicOperationsObserver&, infra::ConstByteRange, infra::Function<void(uint8_t)> onDone)
        {
            writeComplete = onDone;
        });
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    writeComplete(result);
    ExecuteAllActions();
}

TEST_F(GattClientDecoratorTest, should_claim_write_without_response_characteristic_and_release_when_done)
{
    infra::ConstByteRange data = infra::MakeRange(std::array<uint8_t, 4>{ 0x01, 0x02, 0x03, 0x04 });

    claimer.Claim([]() {});
    decorator.WriteWithoutResponse(operationsObserver, data);

    EXPECT_CALL(operations, WriteWithoutResponse(testing::Ref(operationsObserver), infra::ByteRangeContentsEqual(data)));
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    ExecuteAllActions();
}

TEST_F(GattClientDecoratorTest, should_claim_enable_notification_characteristic_and_release_when_done)
{
    const auto result = 123;

    claimer.Claim([]() {});
    decorator.EnableNotification(operationsObserver, infra::MockFunction<void(uint8_t)>(result));

    infra::Function<void(uint8_t)> enableNotificationComplete;
    EXPECT_CALL(operations, EnableNotification(testing::Ref(operationsObserver), testing::_)).WillOnce([&enableNotificationComplete](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(uint8_t)> onDone)
        {
            enableNotificationComplete = onDone;
        });
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    enableNotificationComplete(result);
    ExecuteAllActions();
}

TEST_F(GattClientDecoratorTest, should_claim_disable_notification_characteristic_and_release_when_done)
{
    const auto result = 123;

    claimer.Claim([]() {});
    decorator.DisableNotification(operationsObserver, infra::MockFunction<void(uint8_t)>(result));

    infra::Function<void(uint8_t)> disableNotificationComplete;
    EXPECT_CALL(operations, DisableNotification(testing::Ref(operationsObserver), testing::_)).WillOnce([&disableNotificationComplete](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(uint8_t)> onDone)
        {
            disableNotificationComplete = onDone;
        });
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    disableNotificationComplete(result);
    ExecuteAllActions();
}

TEST_F(GattClientDecoratorTest, should_claim_enable_indication_characteristic_and_release_when_done)
{
    const auto result = 123;

    claimer.Claim([]() {});
    decorator.EnableIndication(operationsObserver, infra::MockFunction<void(uint8_t)>(result));

    infra::Function<void(uint8_t)> enableIndicationComplete;
    EXPECT_CALL(operations, EnableIndication(testing::Ref(operationsObserver), testing::_)).WillOnce([&enableIndicationComplete](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(uint8_t)> onDone)
        {
            enableIndicationComplete = onDone;
        });
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    enableIndicationComplete(result);
    ExecuteAllActions();
}

TEST_F(GattClientDecoratorTest, should_claim_disable_indication_characteristic_and_release_when_done)
{
    const auto result = 123;

    claimer.Claim([]() {});
    decorator.DisableIndication(operationsObserver, infra::MockFunction<void(uint8_t)>(result));

    infra::Function<void(uint8_t)> disableIndicationComplete;
    EXPECT_CALL(operations, DisableIndication(testing::Ref(operationsObserver), testing::_)).WillOnce([&disableIndicationComplete](const services::GattClientCharacteristicOperationsObserver&, infra::Function<void(uint8_t)> onDone)
        {
            disableIndicationComplete = onDone;
        });
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    disableIndicationComplete(result);
    ExecuteAllActions();
}

namespace
{
    class GattClientDiscoveryDecoratorTest
        : public testing::Test
        , public infra::EventDispatcherFixture
    {
    public:
        testing::StrictMock<services::GattClientDiscoveryMock> discovery;
        infra::ClaimableResource resource;
        infra::ClaimableResource::Claimer claimer{ resource };
        infra::MockCallback<void()> mockCallback;
        services::GattClientDiscoveryDecorator decorator{ discovery, resource };
        testing::StrictMock<services::GattClientDiscoveryObserverMock> discoveryObserver{ decorator };
    };
}

TEST_F(GattClientDiscoveryDecoratorTest, should_claim_start_service_discovery_and_release_when_done)
{
    claimer.Claim([]() {});
    decorator.StartServiceDiscovery();

    EXPECT_CALL(discovery, StartServiceDiscovery());
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    EXPECT_CALL(discoveryObserver, ServiceDiscoveryComplete());
    decorator.ServiceDiscoveryComplete();
    ExecuteAllActions();
}

TEST_F(GattClientDiscoveryDecoratorTest, should_claim_start_characteristic_discovery_and_release_when_done)
{
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    claimer.Claim([]() {});
    decorator.StartCharacteristicDiscovery(handle, endHandle);

    EXPECT_CALL(discovery, StartCharacteristicDiscovery(handle, endHandle));
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    EXPECT_CALL(discoveryObserver, CharacteristicDiscoveryComplete());
    decorator.CharacteristicDiscoveryComplete();
    ExecuteAllActions();
}

TEST_F(GattClientDiscoveryDecoratorTest, should_claim_start_descriptor_discovery_and_release_when_done)
{
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    claimer.Claim([]() {});
    decorator.StartDescriptorDiscovery(handle, endHandle);

    EXPECT_CALL(discovery, StartDescriptorDiscovery(handle, endHandle));
    claimer.Release();
    ExecuteAllActions();

    claimer.Claim([this]()
        {
            mockCallback.callback();
        });

    EXPECT_CALL(mockCallback, callback());
    EXPECT_CALL(discoveryObserver, DescriptorDiscoveryComplete());
    decorator.DescriptorDiscoveryComplete();
    ExecuteAllActions();
}

TEST_F(GattClientDiscoveryDecoratorTest, should_discover_service)
{
    const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };
    const auto handle = 0x1;
    const auto endHandle = 0x2;

    std::cout << &type << std::endl;

    EXPECT_CALL(discoveryObserver, ServiceDiscovered(testing::Ref(type), handle, endHandle));
    decorator.ServiceDiscovered(type, handle, endHandle);
}

TEST_F(GattClientDiscoveryDecoratorTest, should_discover_characteristic)
{
    const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };
    const auto handle = 0x1;
    const auto valueHandle = 0x2;
    const auto properties = services::GattCharacteristic::PropertyFlags::read | services::GattCharacteristic::PropertyFlags::notify;

    EXPECT_CALL(discoveryObserver, CharacteristicDiscovered(testing::Ref(type), handle, valueHandle, properties));
    decorator.CharacteristicDiscovered(type, handle, valueHandle, properties);
}

TEST_F(GattClientDiscoveryDecoratorTest, should_discover_descriptor)
{
    const services::AttAttribute::Uuid type = services::AttAttribute::Uuid16{ 0x180D };
    const auto handle = 0x1;

    EXPECT_CALL(discoveryObserver, DescriptorDiscovered(testing::Ref(type), handle));
    decorator.DescriptorDiscovered(type, handle);
}
