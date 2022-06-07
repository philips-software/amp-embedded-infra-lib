#include "services/ble/Gatt.hpp"

namespace services
{
    GattService::GattService(const GattAttribute::Uuid& type)
        : attribute{type, 0}
    {}

    void GattService::AddCharacteristic(const GattCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }

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

    infra::IntrusiveForwardList<GattCharacteristic>& GattService::Characteristics()
    {
        return characteristics;
    }
}
