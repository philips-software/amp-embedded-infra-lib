#include "services/ble/Gatt.hpp"

namespace services
{
    GattDescriptor::GattDescriptor(AttAttribute::Uuid& type, AttAttribute::Handle& handle)
        : type(type)
        , handle(handle)
    {}

    AttAttribute::Uuid& GattDescriptor::Type() const
    {
        return type;
    }

    AttAttribute::Handle& GattDescriptor::Handle() const
    {
        return handle;
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
