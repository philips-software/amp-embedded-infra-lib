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
        MOCK_METHOD(void, ReadCharacteristic, (AttAttribute::Handle, const infra::Function<void(const infra::ConstByteRange&)>&, const infra::Function<void(OperationStatus)>&), (override));
        MOCK_METHOD(void, WriteCharacteristic, (AttAttribute::Handle, infra::ConstByteRange, const infra::Function<void(OperationStatus)>&), (override));
        MOCK_METHOD(void, WriteCharacteristicWithoutResponse, (AttAttribute::Handle, infra::ConstByteRange, const infra::Function<void(OperationStatus)>&), (override));

        MOCK_METHOD(void, ReadDescriptor, (AttAttribute::Handle, const infra::Function<void(const infra::ConstByteRange&)>&, const infra::Function<void(OperationStatus)>&), (override));
        MOCK_METHOD(void, WriteDescriptor, (AttAttribute::Handle, infra::ConstByteRange, const infra::Function<void(OperationStatus)>&), (override));
    };

    class GattClientMtuExchangeMock
        : public GattClientMtuExchange
    {
    public:
        MOCK_METHOD(void, MtuExchange, (const infra::Function<void(OperationStatus)>& onDone), (override));
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

        MOCK_METHOD(void, ReadCharacteristic, (AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, WriteCharacteristic, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, WriteCharacteristicWithoutResponse, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, ReadDescriptor, (AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, WriteDescriptor, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, MtuExchange, (const infra::Function<void(OperationStatus)>& onDone), (override));
    };
}

#endif
