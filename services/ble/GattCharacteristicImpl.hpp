#ifndef SERVICES_GATT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_GATT_CHARACTERISTIC_IMPL_HPP

#include "services/ble/GattClient.hpp"
#include "services/ble/GattServer.hpp"

namespace services
{
    class GattCharacteristicImpl
        : public GattServerCharacteristic
    {
    public:
        using UpdateStatus = GattCharacteristicClientOperations::UpdateStatus;

        GattCharacteristicImpl(AttAttribute attribute, PropertyFlags properties);

        virtual ~GattCharacteristicImpl() = default;

        // Implementation of GattCharacteristic
        virtual PropertyFlags Properties() const;
        virtual uint8_t GetAttributeCount() const;
        virtual AttAttribute::Uuid Type() const;
        virtual AttAttribute::Handle Handle() const;
        virtual AttAttribute::Handle& Handle();
        virtual void Update(infra::ConstByteRange data, infra::Function<void()> onDone);

        // Implementation of GattCharacteristicClientOperationsObserver
        virtual AttAttribute::Handle CharacteristicHandle() const;

    private:
        // Implementation of GattServerCharacteristic
        GattServerCharacteristic::PermissionFlags Permissions() const;
        uint16_t ValueLength() const;

        // Implementation of GattCharacteristicClientOperationsObserver
        virtual AttAttribute::Handle ServiceGroupHandle() const;

    protected:
        void UpdateValue();

    protected:
        struct UpdateContext
        {
            infra::Function<void()> onDone;
            infra::ConstByteRange data;
        };

    protected:
        AttAttribute attribute;
        PropertyFlags properties;
        infra::Optional<UpdateContext> updateContext;
    };

    class GattClientCharacteristicImpl
        : public GattCharacteristicImpl
        , public GattClientCharacteristic
    {
    public:
        using UpdateStatus = GattCharacteristicClientOperations::UpdateStatus;

        GattClientCharacteristicImpl(GattClientService& service, const AttAttribute& attribute);
        GattClientCharacteristicImpl(GattClientService& service, const AttAttribute& attribute, PropertyFlags properties);

        // Implementation of GattCharacteristicClientOperationsObserver
        virtual AttAttribute::Handle ServiceHandle() const;
        virtual AttAttribute::Handle ServiceGroupHandle() const;

        // Implementation of GattClientCharacteristic
        virtual void Read();
        virtual void Write(infra::ConstByteRange data);
        virtual void WriteWithoutResponse(infra::ConstByteRange data);

    private:
        const GattClientService& service;
    };

    class GattServerCharacteristicImpl
        : public GattCharacteristicImpl
    {
    public:
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength);
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties);
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, GattServerCharacteristic::PermissionFlags permissions);

        // Implementation of GattCharacteristic
        virtual void Update(infra::ConstByteRange data, infra::Function<void()> onDone);

        // Implementation of GattServerCharacteristic
        GattServerCharacteristic::PermissionFlags Permissions() const;
        uint16_t ValueLength() const;

        // Implementation of GattCharacteristicClientOperationsObserver
        virtual AttAttribute::Handle ServiceHandle() const;

    private:
        const GattServerService& service;
        uint16_t valueLength;
        PermissionFlags permissions;
    };
}

#endif
