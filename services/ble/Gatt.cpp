#include "services/ble/Gatt.hpp"

namespace services
{
    GattCharacteristic::GattCharacteristic(const Gatt::Uuid& uuid)
        : uuid(uuid)
    {
    }

    GattService::GattService(const Gatt::Uuid& uuid, std::initializer_list<GattCharacteristic> characteristics)
        : uuid(uuid)
    {
        for (auto& c : characteristics)
            AddCharacteristic(c);
    }

    void GattService::AddCharacteristic(const GattCharacteristic& characteristic)
    {
        characteristics.push_back(characteristic);
    }
}
