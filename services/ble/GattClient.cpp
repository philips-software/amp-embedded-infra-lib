#include "services/ble/GattClient.hpp"

namespace services
{
    // GattClientDescriptor::GattClientDescriptor(AttAttribute::Uuid type, AttAttribute::Handle handle)
    //     : type(type)
    //     , handle(handle)
    //{}

    GattClientService::GattClientService(const AttAttribute::Uuid& type)
        : GattService(type, 0, 0)
    {}

    void GattClientService::AddCharacteristic(GattClientCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }

    /* infra::IntrusiveForwardList<GattClientCharacteristic>& GattClientService::Characteristics()
    {
        return characteristics;
    }*/

    const infra::IntrusiveForwardList<GattClientCharacteristic>& GattClientService::Characteristics() const
    {
        return characteristics;
    }
}
