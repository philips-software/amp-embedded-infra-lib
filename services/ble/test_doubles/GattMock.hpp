#ifndef SERVICES_GATT_MOCK_HPP
#define SERVICES_GATT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GattServerMock
        : public GattServer
    {
    public:
        MOCK_METHOD1(AddService, void(GattService& service));
    };

    class GattCharacteristicClientOperationsMock
        : public services::GattCharacteristicClientOperations
    {
    public:
        MOCK_CONST_METHOD2(Update, UpdateStatus(const services::GattCharacteristicClientOperationsObserver& characteristic, infra::ConstByteRange data));
    };

    class GattCharacteristicMock
        : public GattCharacteristic
    {
    public:
        MOCK_CONST_METHOD0(ServiceHandle, GattAttribute::Handle());
        MOCK_CONST_METHOD0(CharacteristicHandle, GattAttribute::Handle());

        MOCK_CONST_METHOD0(Properties, PropertyFlags());
        MOCK_CONST_METHOD0(Permissions, PermissionFlags());
        MOCK_CONST_METHOD0(Type, GattAttribute::Uuid());
        MOCK_CONST_METHOD0(Handle, GattAttribute::Handle());
        MOCK_METHOD0(Handle, GattAttribute::Handle&());
        MOCK_CONST_METHOD0(ValueLength, uint16_t());
        MOCK_METHOD2(Update, void(infra::ConstByteRange data, infra::Function<void()> onDone));
        MOCK_METHOD(uint8_t, GetAttributeCount, (), (const, override));
    };

    class GattCharacteristicUpdateMock
        : public GattCharacteristicUpdate
    {
    public:
        MOCK_METHOD2(Update, void(infra::ConstByteRange data, infra::Function<void()> onDone));
    };
}

#endif
