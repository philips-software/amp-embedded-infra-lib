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

namespace infra
{
    TextOutputStream& operator<<(TextOutputStream& stream, const services::AttAttribute::Uuid& uuid)
    {
        if (uuid.Is<services::AttAttribute::Uuid16>())
            stream << "0x" << hex << uuid.Get<services::AttAttribute::Uuid16>();
        else
            stream << "[" << AsHex(MakeByteRange(uuid.Get<services::AttAttribute::Uuid128>())) << "]";

        return stream;
    }

    TextOutputStream& operator<<(TextOutputStream& stream, const services::GattCharacteristic::PropertyFlags& properties)
    {
        stream << "[";
        if ((properties & services::GattCharacteristic::PropertyFlags::broadcast) != services::GattCharacteristic::PropertyFlags::none)
            stream << "|broadcast|";
        if ((properties & services::GattCharacteristic::PropertyFlags::read) != services::GattCharacteristic::PropertyFlags::none)
            stream << "|read|";
        if ((properties & services::GattCharacteristic::PropertyFlags::writeWithoutResponse) != services::GattCharacteristic::PropertyFlags::none)
            stream << "|writeWithoutResponse|";
        if ((properties & services::GattCharacteristic::PropertyFlags::write) != services::GattCharacteristic::PropertyFlags::none)
            stream << "|write|";
        if ((properties & services::GattCharacteristic::PropertyFlags::notify) != services::GattCharacteristic::PropertyFlags::none)
            stream << "|notify|";
        if ((properties & services::GattCharacteristic::PropertyFlags::indicate) != services::GattCharacteristic::PropertyFlags::none)
            stream << "|indicate|";
        if ((properties & services::GattCharacteristic::PropertyFlags::signedWrite) != services::GattCharacteristic::PropertyFlags::none)
            stream << "|signedWrite|";
        if ((properties & services::GattCharacteristic::PropertyFlags::extended) != services::GattCharacteristic::PropertyFlags::none)
            stream << "|extended|";
        stream << "]";

        return stream;
    }
}
