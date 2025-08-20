#ifndef SERVICES_GATT_CLIENT_MOCK_HPP
#define SERVICES_GATT_CLIENT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GattClientCharacteristicUpdateObserverMock
        : public services::GattClientCharacteristicUpdateObserver
    {
    public:
        using services::GattClientCharacteristicUpdateObserver::GattClientCharacteristicUpdateObserver;

        MOCK_METHOD(void, NotificationReceived, (infra::ConstByteRange data), (override));
        MOCK_METHOD(void, IndicationReceived, (infra::ConstByteRange data, const infra::Function<void()>& onDone), (override));
    };

    class GattClientStackUpdateObserverMock
        : public services::GattClientStackUpdateObserver
    {
    public:
        using services::GattClientStackUpdateObserver::GattClientStackUpdateObserver;

        MOCK_METHOD(void, NotificationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data), (override));
        MOCK_METHOD(void, IndicationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone), (override));
    };

    class GattClientCharacteristicOperationsObserverMock
        : public services::GattClientCharacteristicOperationsObserver
    {
    public:
        using services::GattClientCharacteristicOperationsObserver::GattClientCharacteristicOperationsObserver;

        MOCK_METHOD(AttAttribute::Handle, CharacteristicValueHandle, (), (const, override));
    };

    class GattClientCharacteristicOperationsMock
        : public services::GattClientCharacteristicOperations
    {
    public:
        MOCK_METHOD(void, Read, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void(const infra::ConstByteRange&)>&, const infra::Function<void(uint8_t)>&), (override));
        MOCK_METHOD(void, Write, (const GattClientCharacteristicOperationsObserver&, infra::ConstByteRange, const infra::Function<void(uint8_t)>&), (override));
        MOCK_METHOD(void, WriteWithoutResponse, (const GattClientCharacteristicOperationsObserver&, infra::ConstByteRange), (override));

        MOCK_METHOD(void, EnableNotification, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void(uint8_t)>&), (override));
        MOCK_METHOD(void, DisableNotification, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void(uint8_t)>&), (override));
        MOCK_METHOD(void, EnableIndication, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void(uint8_t)>&), (override));
        MOCK_METHOD(void, DisableIndication, (const GattClientCharacteristicOperationsObserver&, const infra::Function<void(uint8_t)>&), (override));
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

    class GattClientObserverMock
        : public GattClientObserver
    {
    public:
        using GattClientObserver::GattClientObserver;

        MOCK_METHOD(void, ServiceDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, ServiceDiscoveryComplete, (), (override));
        MOCK_METHOD(void, CharacteristicDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties), (override));
        MOCK_METHOD(void, CharacteristicDiscoveryComplete, (), (override));
        MOCK_METHOD(void, DescriptorDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle), (override));
        MOCK_METHOD(void, DescriptorDiscoveryComplete, (), (override));
        MOCK_METHOD(void, NotificationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data), (override));
        MOCK_METHOD(void, IndicationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone), (override));
        MOCK_METHOD(AttAttribute::Handle, CharacteristicValueHandle, (), (const, override));
    };

    class GattClientMock
        : public GattClient
    {
    public:
        MOCK_METHOD(void, StartServiceDiscovery, (), (override));
        MOCK_METHOD(void, StartCharacteristicDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, StartDescriptorDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));

        MOCK_METHOD(void, Read, (const GattClientObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, Write, (const GattClientObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, WriteWithoutResponse, (const GattClientObserver& characteristic, infra::ConstByteRange data), (override));
        MOCK_METHOD(void, EnableNotification, (const GattClientObserver& characteristic, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, DisableNotification, (const GattClientObserver& characteristic, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, EnableIndication, (const GattClientObserver& characteristic, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, DisableIndication, (const GattClientObserver& characteristic, const infra::Function<void(uint8_t)>& onDone), (override));
    };
}

#endif
