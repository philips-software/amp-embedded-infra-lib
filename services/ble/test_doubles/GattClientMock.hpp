#ifndef SERVICES_GATT_MOCK_HPP
#define SERVICES_GATT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"
#include "gmock/gmock.h"

namespace services
{
#if 0
    class GattServerCharacteristicUpdateMock
        : public GattServerCharacteristicUpdate
    {
    public:
        MOCK_METHOD2(Update, void(infra::ConstByteRange data, infra::Function<void()> onDone));
    };
#endif

    class GattClientCharacteristicOperationsMock
        : public services::GattClientCharacteristicOperations
    {
    public:
        MOCK_METHOD(void, Read, (const GattClientCharacteristicOperationsObserver&, infra::Function<void(const infra::ConstByteRange&)>), (const, override));
        MOCK_METHOD(void, Write, (const GattClientCharacteristicOperationsObserver&, infra::ConstByteRange, infra::Function<void()>), (const, override));
        MOCK_METHOD(void, WriteWithoutResponse, (const GattClientCharacteristicOperationsObserver&, infra::ConstByteRange), (const, override));

        MOCK_METHOD(void, EnableNotification, (const GattClientCharacteristicOperationsObserver&, infra::Function<void()>), (const, override));
        MOCK_METHOD(void, DisableNotification, (const GattClientCharacteristicOperationsObserver&, infra::Function<void()>), (const, override));
        MOCK_METHOD(void, EnableIndication, (const GattClientCharacteristicOperationsObserver&, infra::Function<void()>), (const, override));
        MOCK_METHOD(void, DisableIndication, (const GattClientCharacteristicOperationsObserver&, infra::Function<void()>), (const, override));
    };

    class GattClientCharacteristicMock
        : public GattClientCharacteristicOperationsObserver
        , public GattClientDataOperation
        , public GattClientAsyncUpdate
    {
    public:
        // Implementation of GattClientDataOperation
        MOCK_METHOD(void, Read, (infra::Function<void(const infra::ConstByteRange&)> onResponse), (override));
        MOCK_METHOD(void, Write, (infra::ConstByteRange data, infra::Function<void()> onDone), (override));
        MOCK_METHOD(void, WriteWithoutResponse, (infra::ConstByteRange), (override));

        // Implementation of GattClientAsyncUpdate
        MOCK_METHOD(void, EnableNotification, (infra::Function<void()>), (override));
        MOCK_METHOD(void, DisableNotification, (infra::Function<void()>), (override));
        MOCK_METHOD(void, EnableIndication, (infra::Function<void()>), (override));
        MOCK_METHOD(void, DisableIndication, (infra::Function<void()>), (override));

        // Implementation of GattClientCharacteristicOperationsObserver
        MOCK_METHOD(AttAttribute::Handle&, CharacteristicValueHandle, ());
        MOCK_METHOD(GattCharacteristic::PropertyFlags&, CharacteristicProperties, ());
    };

    class GattClientDiscoveryObserverMock
        : public GattClientDiscoveryObserver
    {
        using GattClientDiscoveryObserver::GattClientDiscoveryObserver;

    public:
        MOCK_METHOD(void, ServiceDiscovered, (const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle), (override));
        MOCK_METHOD(void, CharacteristicDiscovered, (const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& valueHandle, const GattCharacteristic::PropertyFlags& properties), (override));
        MOCK_METHOD(void, DescriptorDiscovered, (const AttAttribute::Uuid& type, const AttAttribute::Handle& handle), (override));

        MOCK_METHOD(void, ServiceDiscoveryComplete, (), (override));
        MOCK_METHOD(void, CharacteristicDiscoveryComplete, (), (override));
        MOCK_METHOD(void, DescriptorDiscoveryComplete, (), (override));
    };

    class GattClientDiscoveryMock
        : public GattClientDiscovery
    {
    public:
         MOCK_METHOD(void, StartServiceDiscovery, (), (override));
         MOCK_METHOD(void, StartCharacteristicDiscovery, (const GattService& service), (override));
         MOCK_METHOD(void, StartDescriptorDiscovery, (const GattService& service), (override));
    };
}

#endif
