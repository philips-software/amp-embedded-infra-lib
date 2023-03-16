#include "services/ble/GattClient.hpp"

namespace services
{
    // GattClientDescriptor::GattClientDescriptor(AttAttribute::Uuid type, AttAttribute::Handle handle)
    //     : type(type)
    //     , handle(handle)
    //{}

    GattClientCharacteristic::GattClientCharacteristic(GattClientService& service, const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& valueHandle, const GattCharacteristic::PropertyFlags& properties)
        : GattCharacteristic(type, handle, valueHandle, properties)
        , service(service)
    {
        service.AddCharacteristic(*this);
    }

    GattClientService::GattClientService(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle)
        : GattService(type, handle, endHandle)
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
