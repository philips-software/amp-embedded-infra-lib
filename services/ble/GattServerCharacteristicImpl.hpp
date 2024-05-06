#ifndef SERVICES_GATT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_GATT_CHARACTERISTIC_IMPL_HPP

#include "infra/util/Optional.hpp"
#include "services/ble/GattServer.hpp"

namespace services
{
    class GattServerCharacteristicImpl
        : public GattServerCharacteristic
    {
    public:
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength);
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties);
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, PermissionFlags permissions);
        ~GattServerCharacteristicImpl() override;

        // Implementation of GattServerCharacteristic
        void Update(infra::ConstByteRange data, infra::Function<void()> onDone) override;

        // Implementation of GattServerCharacteristicOperationsObserver
        AttAttribute::Handle ServiceHandle() const override;
        AttAttribute::Handle CharacteristicHandle() const override;

    private:
        struct UpdateContext
        {
            infra::Function<void()> onDone;
            infra::ConstByteRange data;
        };

    private:
        void UpdateValue();

    private:
        GattServerService& service;
        std::optional<UpdateContext> updateContext;
    };
}

#endif
