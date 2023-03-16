#ifndef SERVICES_GATT_CLIENT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_GATT_CLIENT_CHARACTERISTIC_IMPL_HPP

#include "services/ble/GattClient.hpp"

namespace services
{
    class GattClientCharacteristicImpl
        : public GattClientCharacteristicOperationsObserver
        , public GattClientDataOperation
        , public GattClientIndicationNotification
    {
    public:
        GattClientCharacteristicImpl(GattClientCharacteristic& characteristic);

        // Implementation of GattClientCharacteristic
        virtual void Read(infra::Function<void(const infra::ConstByteRange&)> onResponse);
        virtual void Write(infra::ConstByteRange data, infra::Function<void()> onDone);
        virtual void WriteWithoutResponse(infra::ConstByteRange data);

        virtual void EnableNotification(infra::Function<void()> onDone);
        virtual void DisableNotification(infra::Function<void()> onDone);
        virtual void EnableIndication(infra::Function<void()> onDone);
        virtual void DisableIndication(infra::Function<void()> onDone);

        // Implementation of GattClientCharacteristicOperationsObserver
        virtual const AttAttribute::Handle& CharacteristicHandle() const override;
        virtual const GattCharacteristic::PropertyFlags& CharacteristicProperties() const override;

    private:
        GattClientCharacteristic& characteristic;
    };
}

#endif
