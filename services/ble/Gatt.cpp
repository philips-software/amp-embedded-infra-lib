#include "services/ble/Gatt.hpp"

namespace services
{
    GattIdentifier::GattIdentifier(const Gatt::Uuid& type)
        : type(type)
    {}

    const Gatt::Uuid& GattIdentifier::Type() const
    {
        return type;
    }

    Gatt::Handle& GattIdentifier::Handle() const
    {
        return handle;
    }

    void GattService::AddCharacteristic(const GattCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }

    const infra::IntrusiveForwardList<GattCharacteristic>& GattService::Characteristics() const
    {
        return characteristics;
    }
}
