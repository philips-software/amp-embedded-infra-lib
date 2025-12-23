#include "services/ble/ClaimingGattClientAdapter.hpp"
#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"
#include <tuple>

namespace services
{
    ClaimingGattClientAdapter::ClaimingGattClientAdapter(GattClient& gattClient, AttMtuExchange& attMtuExchange, GapCentral& gapCentral)
        : GattClientObserver(gattClient)
        , AttMtuExchangeObserver(attMtuExchange)
        , GapCentralObserver(gapCentral)
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

    void ClaimingGattClientAdapter::ServiceDiscoveryComplete(OperationStatus status)
    {
        discoveryClaimer.Release();
        GattClientDiscovery::NotifyObservers([status](auto& observer)
            {
                observer.ServiceDiscoveryComplete(status);
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

    void ClaimingGattClientAdapter::CharacteristicDiscoveryComplete(OperationStatus status)
    {
        discoveryClaimer.Release();
        GattClientDiscovery::NotifyObservers([status](auto& observer)
            {
                observer.CharacteristicDiscoveryComplete(status);
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

    void ClaimingGattClientAdapter::DescriptorDiscoveryComplete(OperationStatus status)
    {
        discoveryClaimer.Release();
        GattClientDiscovery::NotifyObservers([status](auto& observer)
            {
                observer.DescriptorDiscoveryComplete(status);
            });
    }

    void ClaimingGattClientAdapter::Read(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone)
    {
        characteristicOperationContext.emplace(ReadOperation{ onRead, onDone }, handle);
        characteristicOperationsClaimer.Claim([this]()
            {
                const auto& readContext = std::get<ReadOperation>(characteristicOperationContext->operation);
                GattClientObserver::Subject().Read(characteristicOperationContext->handle, readContext.onRead, [this](OperationStatus result)
                    {
                        characteristicOperationsClaimer.Release();
                        const auto& readContext = std::get<ReadOperation>(characteristicOperationContext->operation);
                        readContext.onDone(result);
                    });
            });
    }

    void ClaimingGattClientAdapter::Write(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        characteristicOperationContext.emplace(WriteOperation{ data, onDone }, handle);
        characteristicOperationsClaimer.Claim([this]()
            {
                const auto& writeContext = std::get<WriteOperation>(characteristicOperationContext->operation);
                GattClientObserver::Subject().Write(characteristicOperationContext->handle, writeContext.data, [this](OperationStatus result)
                    {
                        characteristicOperationsClaimer.Release();
                        const auto& writeContext = std::get<WriteOperation>(characteristicOperationContext->operation);
                        writeContext.onDone(result);
                    });
            });
    }

    void ClaimingGattClientAdapter::WriteWithoutResponse(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        GattClientObserver::Subject().WriteWithoutResponse(handle, data, onDone);
    }

    void ClaimingGattClientAdapter::EnableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone)
    {
        characteristicOperationContext.emplace(DescriptorOperation{ onDone, [this](const infra::Function<void(OperationStatus)>& callback)
                                                   {
                                                       GattClientObserver::Subject().EnableNotification(characteristicOperationContext->handle, callback);
                                                   } },
            handle);

        PerformDescriptorOperation();
    }

    void ClaimingGattClientAdapter::DisableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone)
    {
        characteristicOperationContext.emplace(DescriptorOperation{ onDone,
                                                   [this](const infra::Function<void(OperationStatus)>& callback)
                                                   {
                                                       GattClientObserver::Subject().DisableNotification(characteristicOperationContext->handle, callback);
                                                   } },
            handle);

        PerformDescriptorOperation();
    }

    void ClaimingGattClientAdapter::EnableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone)
    {
        characteristicOperationContext.emplace(DescriptorOperation{ onDone,
                                                   [this](const infra::Function<void(OperationStatus)>& callback)
                                                   {
                                                       GattClientObserver::Subject().EnableIndication(characteristicOperationContext->handle, callback);
                                                   } },
            handle);

        PerformDescriptorOperation();
    }

    void ClaimingGattClientAdapter::DisableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone)
    {
        characteristicOperationContext.emplace(DescriptorOperation{ onDone,
                                                   [this](const infra::Function<void(OperationStatus)>& callback)
                                                   {
                                                       GattClientObserver::Subject().DisableIndication(characteristicOperationContext->handle, callback);
                                                   } },
            handle);

        PerformDescriptorOperation();
    }

    uint16_t ClaimingGattClientAdapter::EffectiveAttMtuSize() const
    {
        return AttMtuExchangeObserver::Subject().EffectiveAttMtuSize();
    }

    void ClaimingGattClientAdapter::MtuExchange(const infra::Function<void(OperationStatus)>& onDone)
    {
        mtuExchangeOnDone = onDone;
        attMtuExchangeClaimer.Claim([this]()
            {
                GattClientObserver::Subject().MtuExchange([this](OperationStatus result)
                    {
                        attMtuExchangeClaimer.Release();
                        mtuExchangeOnDone(result);
                    });
            });
    }

    void ClaimingGattClientAdapter::PerformDescriptorOperation()
    {
        characteristicOperationsClaimer.Claim([this]()
            {
                auto descriptorOperationContext = std::get<DescriptorOperation>(characteristicOperationContext->operation);
                descriptorOperationContext.operation([this](OperationStatus result)
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

    void ClaimingGattClientAdapter::ExchangedAttMtuSize()
    {
        infra::Subject<AttMtuExchangeObserver>::NotifyObservers([](auto& observer)
            {
                observer.ExchangedAttMtuSize();
            });
    }

    void ClaimingGattClientAdapter::DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered)
    {
    }

    void ClaimingGattClientAdapter::StateChanged(GapState state)
    {
        if (state == GapState::standby)
        {
            discoveryClaimer.Release();
            characteristicOperationsClaimer.Release();
            attMtuExchangeClaimer.Release();
        }
    }
}
