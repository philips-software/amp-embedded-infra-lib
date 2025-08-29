#include "services/ble/GattClient.hpp"

namespace services
{
    GattClientCharacteristic::GattClientCharacteristic(GattClientCharacteristicOperations& operations, AttAttribute::Uuid type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties)
        : GattClientCharacteristic(type, handle, valueHandle, properties)
    {
        GattClientCharacteristicOperationsObserver::Attach(operations);
        GattClientStackUpdateObserver::Attach(operations);
    }

    GattClientCharacteristic::GattClientCharacteristic(AttAttribute::Uuid type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties)
        : GattCharacteristic(type, handle, valueHandle, properties)
    {}

    void GattClientCharacteristic::Read(const infra::Function<void(const infra::ConstByteRange&)>& onResponse, const infra::Function<void(uint8_t)>& onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().Read(*this, onResponse, onDone);
    }

    void GattClientCharacteristic::Write(infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().Write(*this, data, onDone);
    }

    void GattClientCharacteristic::WriteWithoutResponse(infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().WriteWithoutResponse(*this, data, onDone);
    }

    void GattClientCharacteristic::EnableNotification(const infra::Function<void(uint8_t)>& onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().EnableNotification(*this, onDone);
    }

    void GattClientCharacteristic::DisableNotification(const infra::Function<void(uint8_t)>& onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().DisableNotification(*this, onDone);
    }

    void GattClientCharacteristic::EnableIndication(const infra::Function<void(uint8_t)>& onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().EnableIndication(*this, onDone);
    }

    void GattClientCharacteristic::DisableIndication(const infra::Function<void(uint8_t)>& onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().DisableIndication(*this, onDone);
    }

    void GattClientCharacteristic::NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data)
    {
        if (handle == (Handle() + GattDescriptor::ClientCharacteristicConfiguration::valueHandleOffset))
            GattClientCharacteristicUpdate::SubjectType::NotifyObservers([&data](auto& obs)
                {
                    obs.NotificationReceived(data);
                });
    }

    void GattClientCharacteristic::IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone)
    {
        if (handle == (Handle() + GattDescriptor::ClientCharacteristicConfiguration::valueHandleOffset))
        {
            onIndicationDone = onDone;
            observers = 1;
            auto indicationReceived = [this]()
            {
                --observers;
                if (observers == 0)
                    onIndicationDone();
            };

            GattClientCharacteristicUpdate::SubjectType::NotifyObservers([this, &data, &indicationReceived](auto& obs)
                {
                    ++observers;
                    obs.IndicationReceived(data, indicationReceived);
                });

            indicationReceived();
        }
        else
            onDone();
    }

    AttAttribute::Handle GattClientCharacteristic::CharacteristicValueHandle() const
    {
        return valueHandle;
    }

    GattCharacteristic::PropertyFlags GattClientCharacteristic::CharacteristicProperties() const
    {
        return properties;
    }

    GattClientService::GattClientService(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle)
        : GattService(type, handle, endHandle)
    {}

    void GattClientService::AddCharacteristic(GattClientCharacteristic& characteristic)
    {
        characteristics.push_front(characteristic);
    }

    const infra::IntrusiveForwardList<GattClientCharacteristic>& GattClientService::Characteristics() const
    {
        return characteristics;
    }

    void GattClientDiscovery::StartCharacteristicDiscovery(const GattService& service)
    {
        StartCharacteristicDiscovery(service.Handle(), service.EndHandle());
    }

    void GattClientDiscovery::StartDescriptorDiscovery(const GattService& service)
    {
        StartDescriptorDiscovery(service.Handle(), service.EndHandle());
    }
}
