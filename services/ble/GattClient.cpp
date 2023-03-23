#include "services/ble/GattClient.hpp"

namespace services
{
    GattClientDescriptor::GattClientDescriptor(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle)
        : GattDescriptor(type, handle)
    {}

    GattClientCharacteristic::GattClientCharacteristic(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& valueHandle, const GattCharacteristic::PropertyFlags& properties)
        : GattCharacteristic(type, handle, valueHandle, properties)
    {}

    void GattClientCharacteristic::AddDescriptor(GattClientDescriptor& descriptor)
    {
        descriptors.push_front(descriptor);
    }

    const infra::IntrusiveForwardList<GattClientDescriptor>& GattClientCharacteristic::Descriptors() const
    {
        return descriptors;
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

    void GattClientDiscoveryDecorator::ServiceDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& endHandle)
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([&type, &handle, &endHandle](auto& obs) { obs.ServiceDiscovered(type, handle, endHandle); });
    }

    void GattClientDiscoveryDecorator::CharacteristicDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle, const AttAttribute::Handle& valueHandle, const GattCharacteristic::PropertyFlags& properties)
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([&type, &handle, &valueHandle, &properties](auto& obs) { obs.CharacteristicDiscovered(type, handle, valueHandle, properties); });
    }

    void GattClientDiscoveryDecorator::DescriptorDiscovered(const AttAttribute::Uuid& type, const AttAttribute::Handle& handle)
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([&type, &handle](auto& obs) { obs.DescriptorDiscovered(type, handle); });
    }

    void GattClientDiscoveryDecorator::ServiceDiscoveryComplete()
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([](auto& obs) { obs.ServiceDiscoveryComplete(); });
    }

    void GattClientDiscoveryDecorator::CharacteristicDiscoveryComplete()
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([](auto& obs) { obs.CharacteristicDiscoveryComplete(); });
    }

    void GattClientDiscoveryDecorator::DescriptorDiscoveryComplete()
    {
        GattClientDiscoveryObserver::SubjectType::NotifyObservers([](auto& obs) { obs.DescriptorDiscoveryComplete(); });
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
