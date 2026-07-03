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
        ClaimingGattClientAdapterMock(GattClient& gattClient, AttMtuExchange& attMtuExchange, GapCentral& gapCentral)
            : ClaimingGattClientAdapter(gattClient, attMtuExchange, gapCentral)
        {}

        MOCK_METHOD(void, StartServiceDiscovery, (), (override));
        MOCK_METHOD(void, StartCharacteristicDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, StartDescriptorDiscovery, (AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, ReadCharacteristic, (AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, WriteCharacteristic, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, WriteCharacteristicWithoutResponse, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, ReadDescriptor, (AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, WriteDescriptor, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone), (override));
        MOCK_METHOD(void, ServiceDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle), (override));
        MOCK_METHOD(void, ServiceDiscoveryComplete, (OperationStatus), (override));
        MOCK_METHOD(void, CharacteristicDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties), (override));
        MOCK_METHOD(void, CharacteristicDiscoveryComplete, (OperationStatus), (override));
        MOCK_METHOD(void, DescriptorDiscovered, (const AttAttribute::Uuid& type, AttAttribute::Handle handle), (override));
        MOCK_METHOD(void, DescriptorDiscoveryComplete, (OperationStatus), (override));
        MOCK_METHOD(void, NotificationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data), (override));
        MOCK_METHOD(void, IndicationReceived, (AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone), (override));
    };
}

#endif
