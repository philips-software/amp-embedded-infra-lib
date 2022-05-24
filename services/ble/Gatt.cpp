#include "services/ble/Gatt.hpp"

namespace services
{
    GattCharacteristic::GattCharacteristic(const Gatt::Uuid& type)
        : type(type)
    {
    }

    GattCharacteristic::GattCharacteristic(infra::ByteRange value, const Gatt::Uuid& type)
        : value(value)
        , type(type)
    {
    }

    GattService::GattService(const Gatt::Uuid& type)
        : type(type)
    {
    }

    void GattService::AddCharacteristic(const GattCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }
}
