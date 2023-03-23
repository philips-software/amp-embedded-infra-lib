#include "services/ble/Gatt.hpp"

namespace services
{
    GattDescriptor::GattDescriptor(const AttAttribute::Uuid& type, AttAttribute::Handle handle)
        : type(type)
        , handle(handle)
    {}

    const AttAttribute::Uuid& GattDescriptor::Type() const
    {
        return type;
    }

    const AttAttribute::Handle& GattDescriptor::Handle() const
    {
        return handle;
    }

    GattCharacteristic::GattCharacteristic(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties)
        : attribute{type, handle, valueHandle}
        , properties(properties)
    {}

    /*GattCharacteristic::PropertyFlags GattCharacteristic::Properties() const
    {
        return properties;
    }*/

    const GattCharacteristic::PropertyFlags& GattCharacteristic::Properties() const
    {
        return properties;
    }

    const AttAttribute::Uuid& GattCharacteristic::Type() const
    {
        return attribute.type;
    }

    const AttAttribute::Handle& GattCharacteristic::Handle() const
    {
        return attribute.handle;
    }

    /*AttAttribute::Handle& GattCharacteristic::Handle()
    {
        return attribute.handle;
    }*/

    const AttAttribute::Handle& GattCharacteristic::ValueHandle() const
    {
        return attribute.endHandle;
    }

    /*AttAttribute::Handle& GattCharacteristic::ValueHandle()
    {
        return attribute.endHandle;
    }*/

    GattService::GattService(const AttAttribute::Uuid& type)
        : GattService(type, 0, 0)
    {}

    GattService::GattService(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle)
        : type(type)
        , handle(handle)
        , endHandle(endHandle)
    {}

    AttAttribute::Uuid GattService::Type() const
    {
        return type;
    }

    AttAttribute::Handle GattService::Handle() const
    {
        return handle;
    }

    AttAttribute::Handle& GattService::Handle()
    {
        return handle;
    }

    AttAttribute::Handle GattService::EndHandle() const
    {
        return endHandle;
    }

    AttAttribute::Handle& GattService::EndHandle()
    {
        return endHandle;
    }

    uint8_t GattService::GetAttributeCount() const
    {
        return 0;
    }
}
