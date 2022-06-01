#include "services/ble/Gatt.hpp"

namespace services
{
    GattIdentifier::GattIdentifier(const Gatt::Uuid& type)
        : type(type)
    {}

    const Gatt::Uuid& GattIdentifier::Type() const
    {
        return type;
    }

    Gatt::Handle& GattIdentifier::Handle()
    {
        return handle;
    }

    Gatt::Handle GattIdentifier::Handle() const
    {
        return handle;
    }

    GattCharacteristic::GattCharacteristic(GattService& service, const Gatt::Uuid& type, uint16_t valueLength)
        : GattIdentifier(type)
        , service(service)
        , valueLength(valueLength)
    {
        service.AddCharacteristic(*this);
    }

    void GattCharacteristic::Update(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(data.size() <= valueLength);

        if (Attached())
            Subject().Update(*this, data);
    }

    Gatt::Handle GattCharacteristic::ServiceHandle() const
    {
        return service.Handle();
    }

    uint16_t GattCharacteristic::ValueLength() const
    {
        return valueLength;
    }

    void GattService::AddCharacteristic(const GattCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }

    infra::IntrusiveForwardList<GattCharacteristic>& GattService::Characteristics()
    {
        return characteristics;
    }
}
