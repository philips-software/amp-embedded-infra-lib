#include "services/ble/ClaimingGattClientAdapter.hpp"
#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"
#include <tuple>

namespace services
{
    ClaimingGattClientAdapter::ClaimingGattClientAdapter(GattClient& gattClient, AttMtuExchange& attMtuExchange)
        : GattClientObserver(gattClient)
        , AttMtuExchangeObserver(attMtuExchange)
    {}

    void ClaimingGattClientAdapter::StartServiceDiscovery()
    {
        discoveryClaimer.Claim([this]()
            {
                GattClientObserver::Subject().StartServiceDiscovery();
            });
    }

    void ClaimingGattClientAdapter::StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        discoveryContext.emplace(HandleRange{ handle, endHandle });
        discoveryClaimer.Claim([this]()
            {
                auto discoveredCharacteristic = std::get<HandleRange>(*discoveryContext);
                GattClientObserver::Subject().StartCharacteristicDiscovery(discoveredCharacteristic.startHandle, discoveredCharacteristic.endHandle);
            });
    }

    void ClaimingGattClientAdapter::StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        discoveryContext.emplace(HandleRange{ handle, endHandle });
        discoveryClaimer.Claim([this]()
            {
                auto discoveredCharacteristic = std::get<HandleRange>(*discoveryContext);
                GattClientObserver::Subject().StartDescriptorDiscovery(discoveredCharacteristic.startHandle, discoveredCharacteristic.endHandle);
            });
    }

    void ClaimingGattClientAdapter::ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        discoveryContext.emplace(DiscoveredService({ type, handle, endHandle }));
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {
                auto discoveredService = std::get<DiscoveredService>(*discoveryContext);
                observer.ServiceDiscovered(discoveredService.type.get(), discoveredService.handle, discoveredService.endHandle);
            });
    }

    void ClaimingGattClientAdapter::ServiceDiscoveryComplete()
    {
        discoveryClaimer.Release();
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.ServiceDiscoveryComplete();
            });
    }

    void ClaimingGattClientAdapter::CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties)
    {
        discoveryContext.emplace(DiscoveredCharacteristic({ type, handle, valueHandle, properties }));
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {
                auto discoveredCharacteristic = std::get<DiscoveredCharacteristic>(*discoveryContext);
                observer.CharacteristicDiscovered(discoveredCharacteristic.type.get(), discoveredCharacteristic.handle, discoveredCharacteristic.valueHandle, discoveredCharacteristic.properties);
            });
    }

    void ClaimingGattClientAdapter::CharacteristicDiscoveryComplete()
    {
        discoveryClaimer.Release();
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.CharacteristicDiscoveryComplete();
            });
    }

    void ClaimingGattClientAdapter::DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle)
    {
        discoveryContext.emplace(DiscoveredDescriptor({ type, handle }));
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {
                auto discoveredDescriptor = std::get<DiscoveredDescriptor>(*discoveryContext);
                observer.DescriptorDiscovered(discoveredDescriptor.type.get(), discoveredDescriptor.handle);
            });
    }

    void ClaimingGattClientAdapter::DescriptorDiscoveryComplete()
    {
        discoveryClaimer.Release();
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.DescriptorDiscoveryComplete();
            });
    }

    void ClaimingGattClientAdapter::Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone)
    {
        characteristicOperationContext.emplace(ReadOperation{ onRead, onDone }, characteristic.CharacteristicValueHandle());
        characteristicOperationsClaimer.Claim([this]()
            {
                auto readContext = std::get<ReadOperation>(characteristicOperationContext->operation);
                GattClientObserver::Subject().Read(*this, readContext.onRead, [this](uint8_t result)
                    {
                        characteristicOperationsClaimer.Release();
                        auto readContext = std::get<ReadOperation>(characteristicOperationContext->operation);
                        readContext.onDone(result);
                    });
            });
    }

    void ClaimingGattClientAdapter::Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone)
    {
        characteristicOperationContext.emplace(WriteOperation{ data, onDone }, characteristic.CharacteristicValueHandle());
        characteristicOperationsClaimer.Claim([this]()
            {
                auto writeContext = std::get<WriteOperation>(characteristicOperationContext->operation);
                GattClientObserver::Subject().Write(*this, writeContext.data, [this](uint8_t result)
                    {
                        characteristicOperationsClaimer.Release();
                        auto writeContext = std::get<WriteOperation>(characteristicOperationContext->operation);
                        writeContext.onDone(result);
                    });
            });
    }

    void ClaimingGattClientAdapter::WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data)
    {
        characteristicOperationContext.emplace(WriteWithoutResponseOperation{ data }, characteristic.CharacteristicValueHandle());
        auto writeWithoutResponseContext = std::get<WriteWithoutResponseOperation>(characteristicOperationContext->operation);
        GattClientObserver::Subject().WriteWithoutResponse(*this, writeWithoutResponseContext.data);
    }

    void ClaimingGattClientAdapter::EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        characteristicOperationContext.emplace(DescriptorOperation{ onDone, [this](const infra::Function<void(uint8_t)>& callback)
                                                   {
                                                       GattClientObserver::Subject().EnableNotification(*this, callback);
                                                   } },
            characteristic.CharacteristicValueHandle());

        PerformDescriptorOperation();
    }

    void ClaimingGattClientAdapter::DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        characteristicOperationContext.emplace(DescriptorOperation{ onDone,
                                                   [this](const infra::Function<void(uint8_t)>& callback)
                                                   {
                                                       GattClientObserver::Subject().DisableNotification(*this, callback);
                                                   } },
            characteristic.CharacteristicValueHandle());

        PerformDescriptorOperation();
    }

    void ClaimingGattClientAdapter::EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        characteristicOperationContext.emplace(DescriptorOperation{ onDone,
                                                   [this](const infra::Function<void(uint8_t)>& callback)
                                                   {
                                                       GattClientObserver::Subject().EnableIndication(*this, callback);
                                                   } },
            characteristic.CharacteristicValueHandle());

        PerformDescriptorOperation();
    }

    void ClaimingGattClientAdapter::DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        characteristicOperationContext.emplace(DescriptorOperation{ onDone,
                                                   [this](const infra::Function<void(uint8_t)>& callback)
                                                   {
                                                       GattClientObserver::Subject().DisableIndication(*this, callback);
                                                   } },
            characteristic.CharacteristicValueHandle());

        PerformDescriptorOperation();
    }

    uint16_t ClaimingGattClientAdapter::EffectiveMaxAttMtuSize() const
    {
        return AttMtuExchangeObserver::Subject().EffectiveMaxAttMtuSize();
    }

    void ClaimingGattClientAdapter::MtuExchange()
    {
        attMtuExchangeClaimer.Claim([this]()
            {
                AttMtuExchangeObserver::Subject().MtuExchange();
            });
    }

    void ClaimingGattClientAdapter::PerformDescriptorOperation()
    {
        characteristicOperationsClaimer.Claim([this]()
            {
                auto descriptorOperationContext = std::get<DescriptorOperation>(characteristicOperationContext->operation);
                descriptorOperationContext.operation([this](uint8_t result)
                    {
                        characteristicOperationsClaimer.Release();
                        auto descriptorOperationContext = std::get<DescriptorOperation>(characteristicOperationContext->operation);
                        descriptorOperationContext.onDone(result);
                    });
            });
    }

    void ClaimingGattClientAdapter::NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data)
    {
        infra::Subject<GattClientStackUpdateObserver>::NotifyObservers([handle, data](auto& observer)
            {
                observer.NotificationReceived(handle, data);
            });
    }

    void ClaimingGattClientAdapter::IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone)
    {
        std::tuple<AttAttribute::Handle, infra::ConstByteRange, infra::Function<void()>> context{ handle, data, onDone };
        infra::Subject<GattClientStackUpdateObserver>::NotifyObservers([&context](auto& observer)
            {
                observer.IndicationReceived(std::get<0>(context), std::get<1>(context), std::get<2>(context));
            });
    }

    AttAttribute::Handle ClaimingGattClientAdapter::CharacteristicValueHandle() const
    {
        return characteristicOperationContext->handle;
    }

    void ClaimingGattClientAdapter::ExchangedMaxAttMtuSize()
    {
        infra::Subject<AttMtuExchangeObserver>::NotifyObservers([](auto& observer)
            {
                observer.ExchangedMaxAttMtuSize();
            });
    }
}
