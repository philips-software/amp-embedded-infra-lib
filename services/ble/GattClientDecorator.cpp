#include "services/ble/GattClientDecorator.hpp"
#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"

namespace services
{
    GattClientDiscoveryDecorator::GattClientDiscoveryDecorator(GattClientDiscovery& discovery, infra::ClaimableResource& gattClientResource)
        : GattClientDiscoveryObserver(discovery)
        , claimer(gattClientResource)
    {}

    void GattClientDiscoveryDecorator::StartServiceDiscovery()
    {
        claimer.Claim([this]()
            {
                GattClientDiscoveryObserver::Subject().StartServiceDiscovery();
            });
    }

    void GattClientDiscoveryDecorator::StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        operationContext.emplace(HandleRange{ handle, endHandle });
        claimer.Claim([this]()
            {
                auto discoveredCharacteristic = std::get<HandleRange>(*operationContext);
                GattClientDiscoveryObserver::Subject().StartCharacteristicDiscovery(discoveredCharacteristic.startHandle, discoveredCharacteristic.endHandle);
            });
    }

    void GattClientDiscoveryDecorator::StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        operationContext.emplace(HandleRange{ handle, endHandle });
        claimer.Claim([this]()
            {
                auto discoveredCharacteristic = std::get<HandleRange>(*operationContext);
                GattClientDiscoveryObserver::Subject().StartDescriptorDiscovery(discoveredCharacteristic.startHandle, discoveredCharacteristic.endHandle);
            });
    }

    void GattClientDiscoveryDecorator::ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle)
    {
        operationContext.emplace(DiscoveredService({ type, handle, endHandle }));
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {
                auto discoveredService = std::get<DiscoveredService>(*operationContext);
                observer.ServiceDiscovered(discoveredService.type.get(), discoveredService.handle, discoveredService.endHandle);
            });
    }

    void GattClientDiscoveryDecorator::ServiceDiscoveryComplete()
    {
        claimer.Release();
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.ServiceDiscoveryComplete();
            });
    }

    void GattClientDiscoveryDecorator::CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties)
    {
        operationContext.emplace(DiscoveredCharacteristic({ type, handle, valueHandle, properties }));
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {
                auto discoveredCharacteristic = std::get<DiscoveredCharacteristic>(*operationContext);
                observer.CharacteristicDiscovered(discoveredCharacteristic.type.get(), discoveredCharacteristic.handle, discoveredCharacteristic.valueHandle, discoveredCharacteristic.properties);
            });
    }

    void GattClientDiscoveryDecorator::CharacteristicDiscoveryComplete()
    {
        claimer.Release();
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.CharacteristicDiscoveryComplete();
            });
    }

    void GattClientDiscoveryDecorator::DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle)
    {
        operationContext.emplace(DiscoveredDescriptor({ type, handle }));
        GattClientDiscovery::NotifyObservers([this](auto& observer)
            {
                auto discoveredDescriptor = std::get<DiscoveredDescriptor>(*operationContext);
                observer.DescriptorDiscovered(discoveredDescriptor.type.get(), discoveredDescriptor.handle);
            });
    }

    void GattClientDiscoveryDecorator::DescriptorDiscoveryComplete()
    {
        claimer.Release();
        GattClientDiscovery::NotifyObservers([](auto& observer)
            {
                observer.DescriptorDiscoveryComplete();
            });
    }

    GattClientCharacteristicOperationsDecorator::GattClientCharacteristicOperationsDecorator(GattClientCharacteristicOperations& operations, infra::ClaimableResource& gattClientResource)
        : GattClientCharacteristicOperationsObserver(operations)
        , claimer(gattClientResource)
    {}

    void GattClientCharacteristicOperationsDecorator::Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone)
    {
        operationContext.emplace(ReadContext{ characteristic, onRead, onDone });
        claimer.Claim([this]()
            {
                auto readContext = std::get<ReadContext>(*operationContext);
                GattClientCharacteristicOperationsObserver::Subject().Read(readContext.characteristic, readContext.onRead, [this](uint8_t result)
                    {
                        claimer.Release();
                        auto readContext = std::get<ReadContext>(*operationContext);
                        readContext.onDone(result);
                    });
            });
    }

    void GattClientCharacteristicOperationsDecorator::Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone)
    {
        operationContext.emplace(WriteContext{ characteristic, data, onDone });
        claimer.Claim([this]()
            {
                auto writeContext = std::get<WriteContext>(*operationContext);
                GattClientCharacteristicOperationsObserver::Subject().Write(writeContext.characteristic, writeContext.data, [this](uint8_t result)
                    {
                        claimer.Release();
                        auto writeContext = std::get<WriteContext>(*operationContext);
                        writeContext.onDone(result);
                    });
            });
    }

    void GattClientCharacteristicOperationsDecorator::WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data)
    {
        operationContext.emplace(WriteWithoutResponseContext{ characteristic, data });
        claimer.Claim([this]()
            {
                auto writeWithoutResponseContext = std::get<WriteWithoutResponseContext>(*operationContext);
                GattClientCharacteristicOperationsObserver::Subject().WriteWithoutResponse(writeWithoutResponseContext.characteristic, writeWithoutResponseContext.data);
                claimer.Release();
            });
    }

    void GattClientCharacteristicOperationsDecorator::EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        operationContext.emplace(DescriptorOperationContext{ characteristic, onDone,
            [this](const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& callback)
            {
                GattClientCharacteristicOperationsObserver::Subject().EnableNotification(characteristic, callback);
            } });

        PerformDescriptorOperation();
    }

    void GattClientCharacteristicOperationsDecorator::DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        operationContext.emplace(DescriptorOperationContext{ characteristic, onDone,
            [this](const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& callback)
            {
                GattClientCharacteristicOperationsObserver::Subject().DisableNotification(characteristic, callback);
            } });

        PerformDescriptorOperation();
    }

    void GattClientCharacteristicOperationsDecorator::EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        operationContext.emplace(DescriptorOperationContext{ characteristic, onDone,
            [this](const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& callback)
            {
                GattClientCharacteristicOperationsObserver::Subject().EnableIndication(characteristic, callback);
            } });

        PerformDescriptorOperation();
    }

    void GattClientCharacteristicOperationsDecorator::DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        operationContext.emplace(DescriptorOperationContext{ characteristic, onDone,
            [this](const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& callback)
            {
                GattClientCharacteristicOperationsObserver::Subject().DisableIndication(characteristic, callback);
            } });

        PerformDescriptorOperation();
    }

    void GattClientCharacteristicOperationsDecorator::PerformDescriptorOperation()
    {
        claimer.Claim([this]()
            {
                auto descriptorOperationContext = std::get<DescriptorOperationContext>(*operationContext);
                descriptorOperationContext.operation(descriptorOperationContext.characteristic, [this](uint8_t result)
                    {
                        claimer.Release();
                        auto descriptorOperationContext = std::get<DescriptorOperationContext>(*operationContext);
                        descriptorOperationContext.onDone(result);
                    });
            });
    }

    AttAttribute::Handle GattClientCharacteristicOperationsDecorator::CharacteristicValueHandle() const
    {
        std::abort();
    }
}
