#ifndef SERVICES_GATT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_GATT_CHARACTERISTIC_IMPL_HPP

#include "infra/util/ByteRange.hpp"
#include "services/ble/GattServer.hpp"
#include <optional>

namespace services
{
    class GattServerCharacteristicImpl
        : public GattServerCharacteristic
    {
    public:
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength);
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties);
        GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, PermissionFlags permissions);
        ~GattServerCharacteristicImpl();

        // Implementation of GattServerCharacteristicUpdate
        void Update(infra::ConstByteRange data, infra::Function<void()> onDone) override;

        // Implementation of GattServerCharacteristicOperationsObserver
        AttAttribute::Handle ServiceHandle() const override;
        AttAttribute::Handle CharacteristicHandle() const override;
        AttAttribute::Handle CharacteristicValueHandle() const override;

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
