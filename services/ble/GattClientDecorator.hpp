#ifndef SERVICES_GATT_CLIENT_DECORATOR_HPP
#define SERVICES_GATT_CLIENT_DECORATOR_HPP

#include "infra/event/ClaimableResource.hpp"
#include "infra/util/Function.hpp"
#include "optional"
#include "services/ble/GattClient.hpp"
#include <cstdint>
#include <functional>
#include <variant>

namespace services
{
    class GattClientDiscoveryDecorator
        : public GattClientDiscovery
        , public GattClientDiscoveryObserver
    {
    public:
        GattClientDiscoveryDecorator(GattClientDiscovery& discovery, infra::ClaimableResource& gattClientResource);

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

        struct HandleRange
        {
            AttAttribute::Handle startHandle;
            AttAttribute::Handle endHandle;
        };

        std::optional<std::variant<DiscoveredService, DiscoveredCharacteristic, DiscoveredDescriptor, HandleRange>> operationContext;
        infra::ClaimableResource::Claimer claimer;
    };

    class GattClientCharacteristicOperationsDecorator
        : public GattClientCharacteristicOperations
        , public GattClientCharacteristicOperationsObserver
    {
    public:
        GattClientCharacteristicOperationsDecorator(GattClientCharacteristicOperations& operations, infra::ClaimableResource& gattClientResource);

        // Implementation of GattClientCharacteristicOperations
        void Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone) override;
        // TODO: fix onRead argument

        void Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone) override;
        void WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data) override;
        void EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;

        // Implementation of GattClientCharacteristicOperationsObserver
        AttAttribute::Handle CharacteristicValueHandle() const override;

    private:
        void PerformDescriptorOperation();

    private:
        struct ReadContext
        {
            std::reference_wrapper<const GattClientCharacteristicOperationsObserver> characteristic;
            const infra::Function<void(const infra::ConstByteRange&)> onRead;
            const infra::Function<void(uint8_t)> onDone;
        };

        struct WriteContext
        {
            std::reference_wrapper<const GattClientCharacteristicOperationsObserver> characteristic;
            infra::ConstByteRange data;
            const infra::Function<void(uint8_t)> onDone;
        };

        struct WriteWithoutResponseContext
        {
            std::reference_wrapper<const GattClientCharacteristicOperationsObserver> characteristic;
            infra::ConstByteRange data;
        };

        struct DescriptorOperationContext
        {
            std::reference_wrapper<const GattClientCharacteristicOperationsObserver> characteristic;
            const infra::Function<void(uint8_t)> onDone;
            infra::Function<void(const GattClientCharacteristicOperationsObserver&, const infra::Function<void(uint8_t)>&)> operation;
        };

        std::optional<std::variant<ReadContext, WriteContext, WriteWithoutResponseContext, DescriptorOperationContext>> operationContext;
        infra::ClaimableResource::Claimer claimer;
    };

    class GattClientDecorator
    {
    public:
        GattClientDecorator(GattClientDiscovery& discovery, GattClientCharacteristicOperations& operations);

        services::GattClientDiscovery& Discovery();
        services::GattClientCharacteristicOperations& CharacteristicOperations();

    private:
        infra::ClaimableResource resource;
        GattClientDiscoveryDecorator discoveryDecorator;
        GattClientCharacteristicOperationsDecorator operationsDecorator;
    };
}

#endif
