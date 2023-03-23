#ifndef SERVICES_GATT_CLIENT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_GATT_CLIENT_CHARACTERISTIC_IMPL_HPP

#include "services/ble/GattClient.hpp"

namespace services
{
    class GattClientCharacteristicImpl
        : public GattClientCharacteristicOperationsObserver
        , public GattClientUpdateObserver
        , public GattClientDataOperation
        , public GattClientUpdate
    {
    public:
        GattClientCharacteristicImpl(const GattClientCharacteristic& characteristic, GattClientCharacteristicOperations& dataOperation);

        // Implementation of GattClientCharacteristic
        virtual void Read(infra::Function<void(const infra::ConstByteRange&)> onResponse) override;
        virtual void Write(infra::ConstByteRange data, infra::Function<void()> onDone) override;
        virtual void WriteWithoutResponse(infra::ConstByteRange data) override;

        virtual void EnableNotification(infra::Function<void()> onDone) override;
        virtual void DisableNotification(infra::Function<void()> onDone) override;
        virtual void EnableIndication(infra::Function<void()> onDone) override;
        virtual void DisableIndication(infra::Function<void()> onDone) override;

        virtual void Update(infra::Function<void(const infra::ConstByteRange&)> onUpdate) override;

        // Implementation of GattClientCharacteristicOperationsObserver
        virtual const AttAttribute::Handle& CharacteristicValueHandle() const override;
        virtual const GattCharacteristic::PropertyFlags& CharacteristicProperties() const override;

    private:
        virtual void ConfigurationCompleted() override;
        virtual void UpdateReceived(const AttAttribute::Handle& handle, infra::ConstByteRange data) override;

    private:
        const GattClientCharacteristic& characteristic;
        infra::Function<void(const infra::ConstByteRange&)> onUpdate;
    };
}

#endif
