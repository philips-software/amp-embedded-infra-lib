#include "services/ble/GattServerCharacteristicImpl.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/PostAssign.hpp"

namespace services
{
    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength)
        : GattServerCharacteristicImpl(service, type, valueLength, PropertyFlags::none, PermissionFlags::none)
    {}

    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties)
        : GattServerCharacteristicImpl(service, type, valueLength, properties, PermissionFlags::none)
    {}

    GattServerCharacteristicImpl::GattServerCharacteristicImpl(GattServerService& service, const AttAttribute::Uuid& type, uint16_t valueLength, PropertyFlags properties, PermissionFlags permissions)
        : GattServerCharacteristic(type, properties, permissions, valueLength)
        , service(service)
    {}

    void GattServerCharacteristicImpl::Update(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(GattServerCharacteristicOperationsObserver::Attached());

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
        auto status = GattServerCharacteristicOperationsObserver::Subject().Update(*this, updateContext->data);

        if (status == GattServerCharacteristicOperations::UpdateStatus::success)
            infra::PostAssign(updateContext, infra::none)->onDone();
        else if (status == GattServerCharacteristicOperations::UpdateStatus::retry)
            infra::EventDispatcher::Instance().Schedule([this]() { UpdateValue(); });
        else
            updateContext = infra::none;
    }
}
