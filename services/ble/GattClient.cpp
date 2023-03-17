#include "services/ble/GattClient.hpp"

namespace services
{
    GattClientDescriptor::GattClientDescriptor(GattClientCharacteristic& characteristic, const AttAttribute::Uuid& type, const AttAttribute::Handle& handle)
        : GattDescriptor(type, handle)
        , characteristic(characteristic)
    {
        characteristic.AddDescriptor(*this);
    }

    GattClientCharacteristic::GattClientCharacteristic(GattClientService& service, const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& valueHandle, const GattCharacteristic::PropertyFlags& properties)
        : GattCharacteristic(type, handle, valueHandle, properties)
        , service(service)
    {
        service.AddCharacteristic(*this);
    }

    void GattClientCharacteristic::AddDescriptor(GattClientDescriptor& descriptor)
    {
        descriptors.push_front(descriptor);
    }

    const infra::IntrusiveForwardList<GattClientDescriptor>& GattClientCharacteristic::Descriptors() const
    {
        return descriptors;
    }

    GattClientService::GattClientService(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle)
        : GattService(type, handle, endHandle)
    {}

    void GattClientService::AddCharacteristic(GattClientCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }

    const infra::IntrusiveForwardList<GattClientCharacteristic>& GattClientService::Characteristics() const
    {
        return characteristics;
    }
}
