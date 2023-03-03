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
}
