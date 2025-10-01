#include "services/ble/GattServer.hpp"

namespace services
{
    GattServerDescriptor::GattServerDescriptor(const AttAttribute::Uuid& type, infra::ConstByteRange data)
        : GattServerDescriptor(type, AccessFlags::readOnly, data)
    {}

    GattServerDescriptor::GattServerDescriptor(const AttAttribute::Uuid& type, const AccessFlags& access, infra::ConstByteRange data)
        : GattDescriptor(type, 0)
        , access(access)
        , data(data)
    {}

    infra::ConstByteRange GattServerDescriptor::Data() const
    {
        return data;
    }

    GattServerDescriptor::AccessFlags GattServerDescriptor::Access() const
    {
        return access;
    }

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

        uint8_t baseAttributeCount;
        if ((properties & (GattCharacteristic::PropertyFlags::notify | GattCharacteristic::PropertyFlags::indicate)) == GattCharacteristic::PropertyFlags::none)
            baseAttributeCount = attributeCountWithoutCCCD;
        else
            baseAttributeCount = attributeCountWithCCCD;

        uint8_t descriptorCount = 0;
        for (const auto& descriptor : descriptors)
        {
            (void)descriptor;
            ++descriptorCount;
        }

        return baseAttributeCount + descriptorCount;
    }

    void GattServerCharacteristic::AddDescriptor(GattServerDescriptor& descriptor)
    {
        descriptors.push_front(descriptor);
    }

    infra::IntrusiveForwardList<GattServerDescriptor>& GattServerCharacteristic::Descriptors()
    {
        return descriptors;
    }

    const infra::IntrusiveForwardList<GattServerDescriptor>& GattServerCharacteristic::Descriptors() const
    {
        return descriptors;
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

    void GattServerService::RemoveCharacteristic(GattServerCharacteristic& characteristic)
    {
        characteristics.erase_slow(characteristic);
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
