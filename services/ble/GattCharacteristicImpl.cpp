#include "services/ble/GattCharacteristicImpl.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/PostAssign.hpp"

namespace services
{
    GattCharacteristicImpl::GattCharacteristicImpl(AttAttribute attribute, PropertyFlags properties)
        : attribute{ attribute }
        , properties{ properties }
    {}

    GattCharacteristic::PropertyFlags GattCharacteristicImpl::Properties() const
    {
        return properties;
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

    AttAttribute::Uuid GattCharacteristicImpl::Type() const
    {
        return attribute.type;
    }

    AttAttribute::Handle GattCharacteristicImpl::Handle() const
    {
        return attribute.handle;
    }

    AttAttribute::Handle& GattCharacteristicImpl::Handle()
    {
        return attribute.handle;
    }

    void GattCharacteristicImpl::Update(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(GattCharacteristicClientOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ onDone, data });
        UpdateValue();
    }

    AttAttribute::Handle GattCharacteristicImpl::CharacteristicHandle() const
    {
        return attribute.handle;
    }

    void GattCharacteristicImpl::UpdateValue()
    {
        auto status = GattCharacteristicClientOperationsObserver::Subject().Update(*this, updateContext->data);

        if (status == UpdateStatus::success)
            infra::PostAssign(updateContext, infra::none)->onDone();
        else if (status == UpdateStatus::retry)
            infra::EventDispatcher::Instance().Schedule([this]() { UpdateValue(); });
        else
            updateContext = infra::none;
    }

    GattServerCharacteristic::PermissionFlags GattCharacteristicImpl::Permissions() const
    {
        return GattServerCharacteristic::PermissionFlags::none;
    }

    uint16_t GattCharacteristicImpl::ValueLength() const
    {
        return 0;
    }

    AttAttribute::Handle GattCharacteristicImpl::ServiceGroupHandle() const
    {
        return 0;
    }

    GattClientCharacteristicImpl::GattClientCharacteristicImpl(GattClientService& service, const AttAttribute& attribute)
        : GattClientCharacteristicImpl(service, attribute, PropertyFlags::none)
    {}

    GattClientCharacteristicImpl::GattClientCharacteristicImpl(GattClientService& service, const AttAttribute& attribute, PropertyFlags properties)
        : GattCharacteristicImpl(attribute, properties)
        , service{ service }
    {
        service.AddCharacteristic(*this);
    }

    AttAttribute::Handle GattClientCharacteristicImpl::ServiceHandle() const
    {
        return service.Handle();
    }

    AttAttribute::Handle GattClientCharacteristicImpl::ServiceGroupHandle() const
    {
        return service.EndHandle();
    }

    void GattClientCharacteristicImpl::Read()
    {
    }

    void GattClientCharacteristicImpl::Write(infra::ConstByteRange data)
    {
    }

    void GattClientCharacteristicImpl::WriteWithoutResponse(infra::ConstByteRange data)
    {
    }

    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength)
        : GattServerCharacteristicImpl(service, type, valueLength, PropertyFlags::none, GattServerCharacteristic::PermissionFlags::none)
    {}

    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties)
        : GattServerCharacteristicImpl(service, type, valueLength, properties, GattServerCharacteristic::PermissionFlags::none)
    {}

    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, GattServerCharacteristic::PermissionFlags permissions)
        : GattCharacteristicImpl({ type, 0, 0 }, properties)
        , service{ service }
        , valueLength(valueLength)
        , permissions(permissions)
    {}

    void GattServerCharacteristicImpl::Update(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(data.size() <= valueLength);
        GattCharacteristicImpl::Update(data, onDone);
    }

    AttAttribute::Handle GattServerCharacteristicImpl::ServiceHandle() const
    {
        return service.Handle();
    }

    GattServerCharacteristic::PermissionFlags GattServerCharacteristicImpl::Permissions() const
    {
        return permissions;
    }

    uint16_t GattServerCharacteristicImpl::ValueLength() const
    {
        return valueLength;
    }
}
