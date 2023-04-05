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
        virtual ~GattServerCharacteristicImpl();

        // Implementation of GattServerCharacteristic
        virtual void Update(infra::ConstByteRange data, infra::Function<void()> onDone);

        // Implementation of GattServerCharacteristicOperationsObserver
        virtual AttAttribute::Handle ServiceHandle() const;
        virtual AttAttribute::Handle CharacteristicHandle() const;

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
        uint16_t valueLength;
        PermissionFlags permissions;
        infra::Optional<UpdateContext> updateContext;
    };
}

#endif
