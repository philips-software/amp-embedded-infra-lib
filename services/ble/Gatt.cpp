#include "services/ble/Gatt.hpp"

namespace services
{
    GattAttribute::Uuid GattService::Type() const
    {
        return attribute.type;
    }

    GattAttribute::Handle GattService::Handle() const
    {
        return attribute.handle;
    }

    GattAttribute::Handle& GattService::Handle()
    {
        return attribute.handle;
    }

    GattService::GattService(const GattAttribute::Uuid& type)
        : attribute{type, 0}
    {}

    void GattService::AddCharacteristic(const GattCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }

    infra::IntrusiveForwardList<GattCharacteristic>& GattService::Characteristics()
    {
        return characteristics;
    }

    const infra::IntrusiveForwardList<GattCharacteristic>& GattService::Characteristics() const
    {
        return characteristics;
    }

    uint8_t GattService::GetAttributeCount() const
    {
        constexpr uint8_t serviceAttributeCount = 1;

        uint8_t attributeCount = serviceAttributeCount;
        for (auto& characteristic : characteristics)
            attributeCount += characteristic.GetAttributeCount();

        return attributeCount;
    }
}
