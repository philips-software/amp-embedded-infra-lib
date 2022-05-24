#include "services/ble/Gatt.hpp"

namespace services
{
    GattCharacteristic::GattCharacteristic(const Gatt::Uuid& type)
        : type(type)
    {
    }

    GattService::GattService(const Gatt::Uuid& type, std::initializer_list<GattCharacteristic> characteristics)
        : type(type)
    {
        for (const auto& c : characteristics)
            AddCharacteristic(c);
    }

    void GattService::AddCharacteristic(const GattCharacteristic& characteristic)
    {
        characteristics.push_back(characteristic);
    }
}
