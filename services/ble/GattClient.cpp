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

    void GattClientCharacteristic::Read(infra::Function<void(const infra::ConstByteRange&)> onResponse)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().Read(*this, onResponse);
    }

    void GattClientCharacteristic::Write(infra::ConstByteRange data, infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().Write(*this, data, onDone);
    }

    void GattClientCharacteristic::WriteWithoutResponse(infra::ConstByteRange data)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().WriteWithoutResponse(*this, data);
    }

    void GattClientCharacteristic::EnableNotification(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().EnableNotification(*this, onDone);
    }

    void GattClientCharacteristic::DisableNotification(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().DisableNotification(*this, onDone);
    }

    void GattClientCharacteristic::EnableIndication(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().EnableIndication(*this, onDone);
    }

    void GattClientCharacteristic::DisableIndication(infra::Function<void()> onDone)
    {
        really_assert(GattClientCharacteristicOperationsObserver::Attached());

        GattClientCharacteristicOperationsObserver::Subject().DisableIndication(*this, onDone);
    }

    void GattClientCharacteristic::UpdateReceived(AttAttribute::Handle handle, infra::ConstByteRange data)
    {
        if (handle == (Handle() + GattDescriptor::ClientCharacteristicConfiguration::valueHandleOffset))
            GattClientCharacteristicUpdate::SubjectType::NotifyObservers([&data](auto& obs)
                { obs.UpdateReceived(data); });
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

    void GattClientDiscoveryDecorator::ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([&type, &handle, &endHandle](auto& obs)
            { obs.ServiceDiscovered(type, handle, endHandle); });
    }

    void GattClientDiscoveryDecorator::CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties)
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([&type, &handle, &valueHandle, &properties](auto& obs)
            { obs.CharacteristicDiscovered(type, handle, valueHandle, properties); });
    }

    void GattClientDiscoveryDecorator::DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle)
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([&type, &handle](auto& obs)
            { obs.DescriptorDiscovered(type, handle); });
    }

    void GattClientDiscoveryDecorator::ServiceDiscoveryComplete()
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([](auto& obs)
            { obs.ServiceDiscoveryComplete(); });
    }

    void GattClientDiscoveryDecorator::CharacteristicDiscoveryComplete()
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([](auto& obs)
            { obs.CharacteristicDiscoveryComplete(); });
    }

    void GattClientDiscoveryDecorator::DescriptorDiscoveryComplete()
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([](auto& obs)
            { obs.DescriptorDiscoveryComplete(); });
    }

    void GattClientDiscoveryDecorator::StartServiceDiscovery()
    {
        GattClientDiscoveryObserver::Subject().StartServiceDiscovery();
    }

    void GattClientDiscoveryDecorator::StartCharacteristicDiscovery(const GattService& service)
    {
        GattClientDiscoveryObserver::Subject().StartCharacteristicDiscovery(service);
    }

    void GattClientDiscoveryDecorator::StartDescriptorDiscovery(const GattService& service)
    {
        GattClientDiscoveryObserver::Subject().StartDescriptorDiscovery(service);
    }
}
