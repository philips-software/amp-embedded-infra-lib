#ifndef SERVICES_CLAIMING_GATT_CLIENT_ADAPTER_MOCK_HPP
#define SERVICES_CLAIMING_GATT_CLIENT_ADAPTER_MOCK_HPP

#include "services/ble/ClaimingGattClientAdapter.hpp"
#include "gmock/gmock.h"

namespace services
{
    class ClaimingGattClientAdapterMock
        : public ClaimingGattClientAdapter
    {
    public:
        ClaimingGattClientAdapterMock(GattClient& gattClient)
            : ClaimingGattClientAdapter(gattClient)
        {}

        MOCK_METHOD(void, StartServiceDiscovery, (), (override));
        MOCK_METHOD(void, StartCharacteristicDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, StartDescriptorDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, Read, (const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, Write, (const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, WriteWithoutResponse, (const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, EnableNotification, (const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, DisableNotification, (const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, EnableIndication, (const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone), (override));
        MOCK_METHOD(void, DisableIndication, (const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone), (override));
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
}

#endif
