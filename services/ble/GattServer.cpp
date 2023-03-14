#include "services/ble/GattServer.hpp"

namespace services
{
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
