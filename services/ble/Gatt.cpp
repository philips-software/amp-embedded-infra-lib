#include "services/ble/Gatt.hpp"

namespace services
{
    GattCharacteristic::GattCharacteristic(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties)
        : type(type)
        , handle(handle)
        , valueHandle(valueHandle)
        , properties(properties)
    {}

    const GattCharacteristic::PropertyFlags& GattCharacteristic::Properties() const
    {
        return properties;
    }

    const AttAttribute::Uuid& GattCharacteristic::Type() const
    {
        return type;
    }

    AttAttribute::Handle GattCharacteristic::Handle() const
    {
        return handle;
    }

    AttAttribute::Handle& GattCharacteristic::Handle()
    {
        return handle;
    }

    AttAttribute::Handle GattCharacteristic::ValueHandle() const
    {
        return valueHandle;
    }

    AttAttribute::Handle& GattCharacteristic::ValueHandle()
    {
        return valueHandle;
    }

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
