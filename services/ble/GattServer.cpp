#include "services/ble/GattServer.hpp"

namespace services
{
    GattServerCharacteristic::GattServerCharacteristic(const AttAttribute::Uuid& type, const PropertyFlags& properties, const PermissionFlags& permissions, uint16_t valueLength)
        : GattCharacteristic(type, 0, 0, properties)
        , permissions(permissions)
        , valueLength(valueLength)
    {}

    GattServerCharacteristic::PermissionFlags GattServerCharacteristic::Permissions() const
    {
        return permissions;
    }

    uint16_t GattServerCharacteristic::ValueLength() const
    {
        return valueLength;
    }

    uint8_t GattServerCharacteristic::GetAttributeCount() const
    {
        constexpr uint8_t attributeCountWithoutCCCD = 2;
        constexpr uint8_t attributeCountWithCCCD = 3;

        if ((properties & (GattCharacteristic::PropertyFlags::notify | GattCharacteristic::PropertyFlags::indicate)) == GattCharacteristic::PropertyFlags::none)
            return attributeCountWithoutCCCD;
        else
            return attributeCountWithCCCD;
    }

    GattServerService::GattServerService(const AttAttribute::Uuid& type)
        : GattService(type, 0, 0)
    {}

    uint8_t GattServerService::GetAttributeCount() const
    {
        constexpr uint8_t serviceAttributeCount = 1;

        uint8_t attributeCount = serviceAttributeCount;
        for (auto& characteristic : characteristics)
            attributeCount += characteristic.GetAttributeCount();

        return attributeCount;
    }

    void GattServerService::AddCharacteristic(GattServerCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }

    infra::IntrusiveForwardList<GattServerCharacteristic>& GattServerService::Characteristics()
    {
        return characteristics;
    }

    const infra::IntrusiveForwardList<GattServerCharacteristic>& GattServerService::Characteristics() const
    {
        return characteristics;
    }
}
