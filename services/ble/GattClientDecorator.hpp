#ifndef SERVICES_GATT_CLIENT_DECORATOR_HPP
#define SERVICES_GATT_CLIENT_DECORATOR_HPP

#include "services/ble/GattClient.hpp"
#include "optional"
#include <functional>
#include <variant>

namespace services
{
    class GattClientDecorator
        : public GattClientDiscovery
        , public GattClientDiscoveryObserver
        , public GattClientCharacteristicOperations
        , public GattClientCharacteristicOperationsObserver
    {
    public:
        GattClientDecorator(GattClientDiscovery& discovery, GattClientCharacteristicOperations& operations)
            : GattClientDiscoveryObserver(discovery)
            , GattClientCharacteristicOperationsObserver(operations) {};

        // Implementation of GattClientDiscovery
        void StartServiceDiscovery() override;
        void StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
        void StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;

        // Implementation of GattClientDiscoveryObserver
        void ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
        void ServiceDiscoveryComplete() override;
        void CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties) override;
        void CharacteristicDiscoveryComplete() override;
        void DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle) override;
        void DescriptorDiscoveryComplete() override;

        // Implementation of GattClientCharacteristicOperations
        void Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone) override;
        //TODO: fix onRead argument

        void Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone) override;
        void WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data) override;
        void EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;

        // Implementation of GattClientCharacteristicOperationsObserver
        AttAttribute::Handle CharacteristicValueHandle() const override;

    private:
        struct DiscoveredService
        {
            std::reference_wrapper<const AttAttribute::Uuid> type;
            AttAttribute::Handle handle;
            AttAttribute::Handle endHandle;
        };

        struct DiscoveredCharacteristic
        {
            std::reference_wrapper<const AttAttribute::Uuid> type;
            AttAttribute::Handle handle;
            AttAttribute::Handle valueHandle;
            GattCharacteristic::PropertyFlags properties;
        };

        struct DiscoveredDescriptor
        {
            std::reference_wrapper<const AttAttribute::Uuid> type;
            AttAttribute::Handle handle;
        };

        std::optional<std::variant<DiscoveredService, DiscoveredCharacteristic, DiscoveredDescriptor>> discoveredItem;
    };
}

#endif
