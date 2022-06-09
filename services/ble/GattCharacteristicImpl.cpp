#include "services/ble/GattCharacteristicImpl.hpp"

namespace services
{
    GattCharacteristicImpl::GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength)
        : GattCharacteristicImpl(service, type, valueLength, Properties::read)
    {}

    GattCharacteristicImpl::GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength, Properties properties)
        : GattCharacteristicImpl(service, type, valueLength, properties, Permissions::none)
    {}

    GattCharacteristicImpl::GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength, Properties properties, Permissions permissions)
        : service{service}
        , attribute{type, 0}
        , valueLength{valueLength}
        , properties{properties}
        , permissions{permissions}
    {
        service.AddCharacteristic(*this);
    }

    GattAttribute::Uuid GattCharacteristicImpl::Type() const
    {
        return attribute.type;
    }

    GattCharacteristic::Properties GattCharacteristicImpl::CharacteristicProperties() const
    {
        return properties;
    }

    GattCharacteristic::Permissions GattCharacteristicImpl::CharacteristicPermissions() const
    {
        return permissions;
    }

    GattAttribute::Handle GattCharacteristicImpl::Handle() const
    {
        return attribute.handle;
    }

    GattAttribute::Handle& GattCharacteristicImpl::Handle()
    {
        return attribute.handle;
    }

    uint16_t GattCharacteristicImpl::ValueLength() const
    {
        return valueLength;
    }

    void GattCharacteristicImpl::Update(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(data.size() <= valueLength);
        really_assert(GattCharacteristicClientOperationsObserver::Attached());

        GattCharacteristicClientOperationsObserver::Subject().Update(*this, data);
        onDone();
    }

    GattAttribute::Handle GattCharacteristicImpl::ServiceHandle() const
    {
        return service.Handle();
    }

    GattAttribute::Handle GattCharacteristicImpl::CharacteristicHandle() const
    {
        return attribute.handle;
    }
}
