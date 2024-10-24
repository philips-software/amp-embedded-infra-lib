#ifndef SERVICES_GATT_MOCK_HPP
#define SERVICES_GATT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GattClientCharacteristicUpdateObserverMock
        : public services::GattClientCharacteristicUpdateObserver
    {
    public:
        MOCK_METHOD(void, UpdateReceived, (infra::ConstByteRange data), (override));
    };

    class GattClientCharacteristicOperationsObserverMock
        : public services::GattClientCharacteristicOperationsObserver
    {
    public:
        MOCK_METHOD(AttAttribute::Handle, CharacteristicValueHandle, (), (const, override));
        MOCK_METHOD(GattCharacteristic::PropertyFlags, CharacteristicProperties, (), (const, override));
    };

    class GattClientCharacteristicOperationsMock
        : public services::GattClientCharacteristicOperations
    {
    public:
        MOCK_METHOD(void, Read, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void(const infra::ConstByteRange&)>&), (override));
        MOCK_METHOD(void, Write, (const GattClientCharacteristicOperationsObserver&, infra::ConstByteRange, const infra::Function<void()>&), (override));
        MOCK_METHOD(void, WriteWithoutResponse, (const GattClientCharacteristicOperationsObserver&, infra::ConstByteRange), (override));

        MOCK_METHOD(void, EnableNotification, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void()>&), (override));
        MOCK_METHOD(void, DisableNotification, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void()>&), (override));
        MOCK_METHOD(void, EnableIndication, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void()>&), (override));
        MOCK_METHOD(void, DisableIndication, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void()>&), (override));
    };

    class GattClientDiscoveryObserverMock
        : public GattClientDiscoveryObserver
    {
        using GattClientDiscoveryObserver::GattClientDiscoveryObserver;

    public:
        MOCK_METHOD(void, ServiceDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, CharacteristicDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties), (override));
        MOCK_METHOD(void, DescriptorDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle), (override));

        MOCK_METHOD(void, ServiceDiscoveryComplete, (), (override));
        MOCK_METHOD(void, CharacteristicDiscoveryComplete, (), (override));
        MOCK_METHOD(void, DescriptorDiscoveryComplete, (), (override));
    };

    class GattClientDiscoveryMock
        : public GattClientDiscovery
    {
    public:
        MOCK_METHOD(void, StartServiceDiscovery, (), (override));
        MOCK_METHOD(void, StartCharacteristicDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, StartDescriptorDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
    };
}

#endif
