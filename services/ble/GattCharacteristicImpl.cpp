#include "services/ble/GattCharacteristicImpl.hpp"

namespace services
{
    GattCharacteristicImpl::GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength)
        : GattCharacteristicImpl(service, type, valueLength, PropertyFlags::read)
    {}

    GattCharacteristicImpl::GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties)
        : GattCharacteristicImpl(service, type, valueLength, properties, PermissionFlags::none)
    {}

    GattCharacteristicImpl::GattCharacteristicImpl(GattService& service, const GattAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, PermissionFlags permissions)
        : service{service}
        , attribute{type, 0}
        , valueLength{valueLength}
        , properties{properties}
        , permissions{permissions}
    {
        service.AddCharacteristic(*this);
    }

    GattCharacteristic::PropertyFlags GattCharacteristicImpl::Properties() const
    {
        return properties;
    }

    GattCharacteristic::PermissionFlags GattCharacteristicImpl::Permissions() const
    {
        return permissions;
    }

     uint8_t GattCharacteristicImpl::GetAttributeCount() const
     {
        constexpr uint8_t attributeCountWithoutCCCD = 2;
        constexpr uint8_t attributeCountWithCCCD = 3;
        
        if ((properties & (GattCharacteristic::PropertyFlags::notify | GattCharacteristic::PropertyFlags::indicate)) == GattCharacteristic::PropertyFlags::none)
            return attributeCountWithoutCCCD;
        else 
            return attributeCountWithCCCD;
     }

    GattAttribute::Uuid GattCharacteristicImpl::Type() const
    {
        return attribute.type;
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
        
        while (!GattCharacteristicClientOperationsObserver::Subject().Update(*this, data));
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
