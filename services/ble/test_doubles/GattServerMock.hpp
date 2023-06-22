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
        MOCK_METHOD1(AddService, void(GattServerService& service));
    };

    class GattServerCharacteristicOperationsMock
        : public services::GattServerCharacteristicOperations
    {
    public:
        MOCK_CONST_METHOD2(Update, UpdateStatus(const services::GattServerCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data));
    };

    class GattServerCharacteristicUpdateMock
        : public GattServerCharacteristicUpdate
    {
    public:
        MOCK_METHOD2(Update, void(infra::ConstByteRange data, infra::Function<void()> onDone));
    };

    class GattServerCharacteristicMock
        : public GattServerCharacteristic
    {
    public:
        MOCK_CONST_METHOD0(ServiceHandle, AttAttribute::Handle());
        MOCK_CONST_METHOD0(CharacteristicHandle, AttAttribute::Handle());

        MOCK_METHOD2(Update, void(infra::ConstByteRange data, infra::Function<void()> onDone));
    };
}

#endif
