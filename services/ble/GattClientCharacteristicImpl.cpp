#include "services/ble/GattClientCharacteristicImpl.hpp"
#include "infra/event/EventDispatcher.hpp"
#include "infra/util/PostAssign.hpp"

namespace services
{
    GattClientCharacteristicImpl::GattClientCharacteristicImpl(const GattClientCharacteristic& characteristic, GattClientCharacteristicOperations& dataOperation)
        : GattClientCharacteristicOperationsObserver(dataOperation)
        , characteristic(characteristic)
    {}

    void GattClientCharacteristicImpl::Read(infra::Function<void(const infra::ConstByteRange&)> onResponse)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().Read(*this, onResponse);
    }

    void GattClientCharacteristicImpl::Write(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().Write(*this, data, onDone);
    }

    void GattClientCharacteristicImpl::WriteWithoutResponse(infra::ConstByteRange data)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().WriteWithoutResponse(*this, data);
    }

    void GattClientCharacteristicImpl::EnableNotification(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().EnableNotification(*this, onDone);
    }

    void GattClientCharacteristicImpl::DisableNotification(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().DisableNotification(*this, onDone);
    }

    void GattClientCharacteristicImpl::EnableIndication(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().EnableIndication(*this, onDone);
    }

    void GattClientCharacteristicImpl::DisableIndication(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().DisableIndication(*this, onDone);
    }

    void GattClientCharacteristicImpl::Update(infra::Function<void(const infra::ConstByteRange&)> onUpdate)
    {
        this->onUpdate = onUpdate;
    }

    const AttAttribute::Handle& GattClientCharacteristicImpl::CharacteristicHandle() const
    {
        return characteristic.Handle();
    }

    const GattCharacteristic::PropertyFlags& GattClientCharacteristicImpl::CharacteristicProperties() const
    {
        return characteristic.Properties();
    }

    void GattClientCharacteristicImpl::ConfigurationCompleted()
    {}

    void GattClientCharacteristicImpl::UpdateReceived(const AttAttribute::Handle& handle, infra::ConstByteRange data)
    {
        if (handle == characteristic.Handle())
        {
            this->onUpdate(data);
        }
    }
}
