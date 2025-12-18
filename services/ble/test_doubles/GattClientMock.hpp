#ifndef SERVICES_GATT_CLIENT_MOCK_HPP
#define SERVICES_GATT_CLIENT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GattClientCharacteristicUpdateObserverMock
        : public GattClientCharacteristicUpdateObserver
    {
    public:
        using GattClientCharacteristicUpdateObserver::GattClientCharacteristicUpdateObserver;

        MOCK_METHOD(void, NotificationReceived, (infra::ConstByteRange data), (override));
        MOCK_METHOD(void, IndicationReceived, (infra::ConstByteRange data, const infra::Function<void()>& onDone), (override));
    };

    class GattClientStackUpdateObserverMock
        : public GattClientStackUpdateObserver
    {
    public:
        using GattClientStackUpdateObserver::GattClientStackUpdateObserver;

        MOCK_METHOD(void, NotificationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data), (override));
        MOCK_METHOD(void, IndicationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone), (override));
    };

    class GattClientCharacteristicOperationsMock
        : public GattClientCharacteristicOperations
    {
    public:
        MOCK_METHOD(void, Read, (AttAttribute::Handle, const infra::Function<void(const infra::ConstByteRange&)>&, const infra::Function<void(OperationStatus)>&), (override));
        MOCK_METHOD(void, Write, (AttAttribute::Handle, infra::ConstByteRange, const infra::Function<void(OperationStatus)>&), (override));
        MOCK_METHOD(void, WriteWithoutResponse, (AttAttribute::Handle, infra::ConstByteRange, const infra::Function<void(OperationStatus)>&), (override));

        MOCK_METHOD(void, EnableNotification, (AttAttribute::Handle, const infra::Function<void(OperationStatus)>&), (override));
        MOCK_METHOD(void, DisableNotification, (AttAttribute::Handle, const infra::Function<void(OperationStatus)>&), (override));
        MOCK_METHOD(void, EnableIndication, (AttAttribute::Handle, const infra::Function<void(OperationStatus)>&), (override));
        MOCK_METHOD(void, DisableIndication, (AttAttribute::Handle, const infra::Function<void(OperationStatus)>&), (override));
    };

    class GattClientDiscoveryObserverMock
        : public GattClientDiscoveryObserver
    {
        using GattClientDiscoveryObserver::GattClientDiscoveryObserver;

    public:
        MOCK_METHOD(void, ServiceDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, CharacteristicDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties), (override));
        MOCK_METHOD(void, DescriptorDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle), (override));

        MOCK_METHOD(void, ServiceDiscoveryComplete, (OperationStatus status), (override));
        MOCK_METHOD(void, CharacteristicDiscoveryComplete, (OperationStatus status), (override));
        MOCK_METHOD(void, DescriptorDiscoveryComplete, (OperationStatus status), (override));
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
        MOCK_METHOD(void, ServiceDiscoveryComplete, (OperationStatus status), (override));
        MOCK_METHOD(void, CharacteristicDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties), (override));
        MOCK_METHOD(void, CharacteristicDiscoveryComplete, (OperationStatus status), (override));
        MOCK_METHOD(void, DescriptorDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle), (override));
        MOCK_METHOD(void, DescriptorDiscoveryComplete, (OperationStatus status), (override));
        MOCK_METHOD(void, NotificationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data), (override));
        MOCK_METHOD(void, IndicationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone), (override));
    };

    class GattClientMock
        : public GattClient
    {
    public:
        MOCK_METHOD(void, StartServiceDiscovery, (), (override));
        MOCK_METHOD(void, StartCharacteristicDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, StartDescriptorDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));

        MOCK_METHOD(void, Read, (AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, Write, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, WriteWithoutResponse, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, EnableNotification, (AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, DisableNotification, (AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, EnableIndication, (AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, DisableIndication, (AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, MtuExchange, (const infra::Function<void(OperationStatus)>& onDone), (override));
    };
}

#endif
