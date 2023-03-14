#include "services/ble/GattCharacteristicImpl.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/PostAssign.hpp"

namespace services
{
    GattCharacteristicImpl::GattCharacteristicImpl(const AttAttribute::Uuid& type)
        : GattCharacteristicImpl(type, PropertyFlags::none)
    {}

    GattCharacteristicImpl::GattCharacteristicImpl(const AttAttribute::Uuid& type, PropertyFlags properties)
        : attribute{ attribute }
        , properties{ properties }
    {}

    GattCharacteristic::PropertyFlags GattCharacteristicImpl::Properties() const
    {
        return properties;
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

    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength)
        : GattServerCharacteristicImpl(service, type, valueLength, PropertyFlags::none, PermissionFlags::none)
    {}

    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties)
        : GattServerCharacteristicImpl(service, type, valueLength, properties, PermissionFlags::none)
    {}

    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, PermissionFlags permissions)
        : GattCharacteristicImpl(type, properties)
        , service(service)
        , valueLength(valueLength)
        , permissions(permissions)
    {}

    GattCharacteristic::PropertyFlags GattServerCharacteristicImpl::Properties() const
    {
        return GattCharacteristicImpl::Properties();
    }

    AttAttribute::Uuid GattServerCharacteristicImpl::Type() const
    {
        return GattCharacteristicImpl::Type();
    }

    AttAttribute::Handle GattServerCharacteristicImpl::Handle() const
    {
        return GattCharacteristicImpl::Handle();
    }

    AttAttribute::Handle& GattServerCharacteristicImpl::Handle()
    {
        return GattCharacteristicImpl::Handle();
    }

    GattServerCharacteristic::PermissionFlags GattServerCharacteristicImpl::Permissions() const
    {
        return permissions;
    }

    uint16_t GattServerCharacteristicImpl::ValueLength() const
    {
        return valueLength;
    }

    uint8_t GattServerCharacteristicImpl::GetAttributeCount() const
    {
        constexpr uint8_t attributeCountWithoutCCCD = 2;
        constexpr uint8_t attributeCountWithCCCD = 3;

        if ((properties & (GattCharacteristic::PropertyFlags::notify | GattCharacteristic::PropertyFlags::indicate)) == GattCharacteristic::PropertyFlags::none)
            return attributeCountWithoutCCCD;
        else
            return attributeCountWithCCCD;
    }

    void GattServerCharacteristicImpl::Update(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(GattCharacteristicClientOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ onDone, data });
        UpdateValue();
    }

    AttAttribute::Handle GattServerCharacteristicImpl::ServiceHandle() const
    {
        return service.Handle();
    }

    AttAttribute::Handle GattServerCharacteristicImpl::CharacteristicHandle() const
    {
        return attribute.handle;
    }

    void GattServerCharacteristicImpl::UpdateValue()
    {
        auto status = GattCharacteristicClientOperationsObserver::Subject().Update(*this, updateContext->data);

        if (status == GattCharacteristicClientOperations::UpdateStatus::success)
            infra::PostAssign(updateContext, infra::none)->onDone();
        else if (status == GattCharacteristicClientOperations::UpdateStatus::retry)
            infra::EventDispatcher::Instance().Schedule([this]() { UpdateValue(); });
        else
            updateContext = infra::none;
    }

    GattClientCharacteristicImpl::GattClientCharacteristicImpl(GattClientService& service, const AttAttribute::Uuid& type)
        : GattClientCharacteristicImpl(service, type, PropertyFlags::none)
    {}

    GattClientCharacteristicImpl::GattClientCharacteristicImpl(GattClientService& service, const AttAttribute::Uuid& type, PropertyFlags properties)
        : GattCharacteristicImpl(type, properties)
        , service(service)
    {}

    GattCharacteristic::PropertyFlags GattClientCharacteristicImpl::Properties() const
    {
        return GattCharacteristicImpl::Properties();
    }

    AttAttribute::Uuid GattClientCharacteristicImpl::Type() const
    {
        return GattCharacteristicImpl::Type();
    }

    AttAttribute::Handle GattClientCharacteristicImpl::Handle() const
    {
        return GattCharacteristicImpl::Handle();
    }

    AttAttribute::Handle& GattClientCharacteristicImpl::Handle()
    {
        return GattCharacteristicImpl::Handle();
    }

    void GattClientCharacteristicImpl::Read(infra::Function<void(infra::ConstByteRange&)> onResponse)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ {}, {}, onResponse });

        if (GattClientCharacteristicOperationsObserver::Subject().Read(*this))
            infra::PostAssign(updateContext, infra::none)->onDone();
        else
            updateContext = infra::none;
    }

    void GattClientCharacteristicImpl::Write(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ onDone, data });

        if (GattClientCharacteristicOperationsObserver::Subject().Write(*this, updateContext->data))
            infra::PostAssign(updateContext, infra::none)->onDone();
        else
            updateContext = infra::none;
    }

    void GattClientCharacteristicImpl::WriteWithoutResponse(infra::ConstByteRange data)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ {}, data });

        GattClientCharacteristicOperationsObserver::Subject().WriteWithoutResponse(*this, updateContext->data);

        updateContext = infra::none;
    }

    void GattClientCharacteristicImpl::EnableNotification(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ onDone, {} });

        if (GattClientCharacteristicOperationsObserver::Subject().EnableNotification(*this))
            infra::PostAssign(updateContext, infra::none)->onDone();
        else
            updateContext = infra::none;
    }

    void GattClientCharacteristicImpl::DisableNotification(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ onDone, {} });

        if (GattClientCharacteristicOperationsObserver::Subject().DisableNotification(*this))
            infra::PostAssign(updateContext, infra::none)->onDone();
        else
            updateContext = infra::none;
    }

    void GattClientCharacteristicImpl::EnableIndication(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ onDone, {} });

        if (GattClientCharacteristicOperationsObserver::Subject().EnableIndication(*this))
            infra::PostAssign(updateContext, infra::none)->onDone();
        else
            updateContext = infra::none;
    }

    void GattClientCharacteristicImpl::DisableIndication(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        updateContext.Emplace(UpdateContext{ onDone, {} });

        if (GattClientCharacteristicOperationsObserver::Subject().DisableIndication(*this))
            infra::PostAssign(updateContext, infra::none)->onDone();
        else
            updateContext = infra::none;
    }

    AttAttribute::Handle GattClientCharacteristicImpl::CharacteristicHandle() const
    {
        return attribute.handle;
    }
}
