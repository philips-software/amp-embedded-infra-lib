#include "services/ble/GattClientDecorator.hpp"
#include "infra/util/PostAssign.hpp"
#include "iostream"
#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"

namespace services
{
    void GattClientDecorator::StartServiceDiscovery()
    {
        GattClientDiscoveryObserver::Subject().StartServiceDiscovery();
    }

    void GattClientDecorator::StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        GattClientDiscoveryObserver::Subject().StartCharacteristicDiscovery(handle, endHandle);
    }

    void GattClientDecorator::StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        GattClientDiscoveryObserver::Subject().StartDescriptorDiscovery(handle, endHandle);
    }

    void GattClientDecorator::ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        discoveredItem = DiscoveredService({ type, handle, endHandle });
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {                
                auto discoveredService = std::get<DiscoveredService>(*(infra::PostAssign(discoveredItem, std::nullopt)));
                observer.ServiceDiscovered(discoveredService.type.get(), discoveredService.handle, discoveredService.endHandle);
            });
    }

    void GattClientDecorator::ServiceDiscoveryComplete()
    {
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.ServiceDiscoveryComplete();
            });
    }

    void GattClientDecorator::CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties)
    {
        discoveredItem = DiscoveredCharacteristic({ type, handle, valueHandle, properties });
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {
                auto discoveredCharacteristic = std::get<DiscoveredCharacteristic>(*(infra::PostAssign(discoveredItem, std::nullopt)));
                observer.CharacteristicDiscovered(discoveredCharacteristic.type.get(), discoveredCharacteristic.handle, discoveredCharacteristic.valueHandle, discoveredCharacteristic.properties);
            });
    }

    void GattClientDecorator::CharacteristicDiscoveryComplete()
    {
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.CharacteristicDiscoveryComplete();
            });
    }

    void GattClientDecorator::DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle)
    {
        discoveredItem = DiscoveredDescriptor({ type, handle });
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {
                auto discoveredDescriptor = std::get<DiscoveredDescriptor>(*(infra::PostAssign(discoveredItem, std::nullopt)));
                observer.DescriptorDiscovered(discoveredDescriptor.type.get(), discoveredDescriptor.handle);
            });
    }

    void GattClientDecorator::DescriptorDiscoveryComplete()
    {
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.DescriptorDiscoveryComplete();
            });
    }

    void GattClientDecorator::Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone)
    {
        GattClientCharacteristicOperationsObserver::Subject().Read(characteristic, onRead, onDone);
    }

    void GattClientDecorator::Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone)
    {
        GattClientCharacteristicOperationsObserver::Subject().Write(characteristic, data, onDone);
    }

    void GattClientDecorator::WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data)
    {
        GattClientCharacteristicOperationsObserver::Subject().WriteWithoutResponse(characteristic, data);
    }

    void GattClientDecorator::EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        GattClientCharacteristicOperationsObserver::Subject().EnableNotification(characteristic, onDone);
    }

    void GattClientDecorator::DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        GattClientCharacteristicOperationsObserver::Subject().DisableNotification(characteristic, onDone);
    }

    void GattClientDecorator::EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        GattClientCharacteristicOperationsObserver::Subject().EnableIndication(characteristic, onDone);
    }

    void GattClientDecorator::DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        GattClientCharacteristicOperationsObserver::Subject().DisableIndication(characteristic, onDone);
    }

    AttAttribute::Handle GattClientDecorator::CharacteristicValueHandle() const
    {
        std::abort();
    }
}
