#ifndef SERVICES_GATT_MOCK_HPP
#define SERVICES_GATT_MOCK_HPP

#include "gmock/gmock.h"
#include "services/ble/Gatt.hpp"

namespace services
{
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
    };
}

#endif
