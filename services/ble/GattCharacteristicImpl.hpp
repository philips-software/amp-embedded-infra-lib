#ifndef SERVICES_GATT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_GATT_CHARACTERISTIC_IMPL_HPP

#include "services/ble/GattClient.hpp"
#include "services/ble/GattServer.hpp"

namespace services
{
    class GattCharacteristicImpl
        : public GattCharacteristic
    {
    public:
        GattCharacteristicImpl(const AttAttribute::Uuid& type);
        GattCharacteristicImpl(const AttAttribute::Uuid& type, PropertyFlags properties);

        // Implementation of GattCharacteristic
        virtual PropertyFlags Properties() const;
        virtual AttAttribute::Uuid Type() const;
        virtual AttAttribute::Handle Handle() const;
        virtual AttAttribute::Handle& Handle();

    protected:
        struct UpdateContext
        {
            infra::Function<void()> onDone;
            infra::ConstByteRange data;
            infra::Function<void(infra::ConstByteRange&)> onResponse;
        };

    protected:
        AttAttribute attribute;
        PropertyFlags properties;
        infra::Optional<UpdateContext> updateContext;
    };

    class GattServerCharacteristicImpl
        : public GattCharacteristicImpl
        , public GattServerCharacteristic
    {
    public:
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength);
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties);
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, PermissionFlags permissions);

        // Implementation of GattCharacteristic
        virtual PropertyFlags Properties() const;
        virtual AttAttribute::Uuid Type() const;
        virtual AttAttribute::Handle Handle() const;
        virtual AttAttribute::Handle& Handle();

        // Implementation of GattServerCharacteristic
        virtual PermissionFlags Permissions() const;
        virtual uint16_t ValueLength() const;
        virtual uint8_t GetAttributeCount() const;
        virtual void Update(infra::ConstByteRange data, infra::Function<void()> onDone);

        // Implementation of GattCharacteristicClientOperationsObserver
        virtual AttAttribute::Handle ServiceHandle() const;
        virtual AttAttribute::Handle CharacteristicHandle() const;

    private:
        void UpdateValue();

    private:
        const GattServerService& service;
        uint16_t valueLength;
        PermissionFlags permissions;
    };

    class GattClientCharacteristicImpl
        : public GattCharacteristicImpl
        , public GattClientCharacteristic
    {
    public:
        GattClientCharacteristicImpl(GattClientService& service, const AttAttribute::Uuid& type);
        GattClientCharacteristicImpl(GattClientService& service, const AttAttribute::Uuid& type, PropertyFlags properties);

        // Implementation of GattCharacteristic
        virtual PropertyFlags Properties() const;
        virtual AttAttribute::Uuid Type() const;
        virtual AttAttribute::Handle Handle() const;
        virtual AttAttribute::Handle& Handle();

        // Implementation of GattClientCharacteristic
        virtual void Read(infra::Function<void(infra::ConstByteRange&)> onResponse);
        virtual void Write(infra::ConstByteRange data, infra::Function<void()> onDone);
        virtual void WriteWithoutResponse(infra::ConstByteRange data);

        virtual void EnableNotification(infra::Function<void()> onDone);
        virtual void DisableNotification(infra::Function<void()> onDone);
        virtual void EnableIndication(infra::Function<void()> onDone);
        virtual void DisableIndication(infra::Function<void()> onDone);

        // Implementation of GattCharacteristicClientOperationsObserver
        virtual AttAttribute::Handle CharacteristicHandle() const;

    private:
        const GattClientService& service;
    };
}

#endif
