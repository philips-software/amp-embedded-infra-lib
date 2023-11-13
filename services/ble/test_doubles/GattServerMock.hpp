#ifndef SERVICES_GATT_MOCK_HPP
#define SERVICES_GATT_MOCK_HPP

#include "services/ble/Gatt.hpp"
#include "services/ble/GattServer.hpp"
#include "gmock/gmock.h"

namespace services
{
    class GattServerMock
        : public GattServer
    {
    public:
        MOCK_METHOD(void, AddService, (GattServerService& service));
    };

    class GattServerCharacteristicOperationsMock
        : public services::GattServerCharacteristicOperations
    {
    public:
        MOCK_METHOD(UpdateStatus, Update, (const services::GattServerCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data), (const));
    };

    class GattServerCharacteristicUpdateMock
        : public GattServerCharacteristicUpdate
    {
    public:
        MOCK_METHOD(void, Update, (infra::ConstByteRange data, infra::Function<void()> onDone));
    };

    class GattServerCharacteristicMock
        : public GattServerCharacteristic
    {
    public:
        MOCK_METHOD(AttAttribute::Handle, ServiceHandle, (), (const));
        MOCK_METHOD(AttAttribute::Handle, CharacteristicHandle, (), (const));
        MOCK_METHOD(void, Update, (infra::ConstByteRange data, infra::Function<void()> onDone));
    };
}

#endif