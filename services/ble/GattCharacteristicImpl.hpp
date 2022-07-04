#ifndef SERVICES_GATT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_GATT_CHARACTERISTIC_IMPL_HPP

#include "services/ble/Gatt.hpp"

namespace services
{
    class GattCharacteristicImpl
        : public GattCharacteristic
    {
    public:
        GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength);
        GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties);
        GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, PermissionFlags permissions);

        // Implementation of GattCharacteristic
        virtual PropertyFlags Properties() const;
        virtual PermissionFlags Permissions() const;
        virtual uint8_t GetAttributeCount() const;
        virtual GattAttribute::Uuid Type() const;
        virtual GattAttribute::Handle Handle() const;
        virtual GattAttribute::Handle& Handle();
        virtual uint16_t ValueLength() const;
        virtual void Update(infra::ConstByteRange data, infra::Function<void()> onDone);

        // Implementation of GattCharacteristicClientOperationsObserver
        virtual GattAttribute::Handle ServiceHandle() const;
        virtual GattAttribute::Handle CharacteristicHandle() const;

    private:
        const GattService& service;
        GattAttribute attribute;
        uint16_t valueLength;
        PropertyFlags properties;
        PermissionFlags permissions;
    };
}

#endif
