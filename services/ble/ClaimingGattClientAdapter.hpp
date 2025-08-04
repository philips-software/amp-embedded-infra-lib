#ifndef SERVICES_GATT_CLIENT_DECORATOR_HPP
#define SERVICES_GATT_CLIENT_DECORATOR_HPP

#include "infra/event/ClaimableResource.hpp"
#include "infra/util/Function.hpp"
#include "optional"
#include "services/ble/Gatt.hpp"
#include "services/ble/GattClient.hpp"
#include <cstdint>
#include <functional>
#include <variant>

namespace services
{
    class ClaimingGattClientAdapter
        : public GattClientDiscovery
        , public GattClientCharacteristicOperations
        , public AttMtuExchange
        , private GattClientObserver
        , private AttMtuExchangeObserver
    {
    public:
        ClaimingGattClientAdapter(GattClient& gattClient, AttMtuExchange& attMtuExchange);

        // Implementation of GattClientDiscovery
        void StartServiceDiscovery() override;
        void StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
        void StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;

        // Implementation of GattClientCharacteristicOperations
        void Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone) override;
        void Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone) override;
        void WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data) override;
        void EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;

        // Implementation of AttMtuExchange
        uint16_t EffectiveMaxAttMtuSize() const override;
        void MtuExchange() const override;

    private:
        // Implementation of GattClientObserver
        void ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
        void ServiceDiscoveryComplete() override;
        void CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties) override;
        void CharacteristicDiscoveryComplete() override;
        void DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle) override;
        void DescriptorDiscoveryComplete() override;
        void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) override;
        void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) override;
        AttAttribute::Handle CharacteristicValueHandle() const override;

        // Implementation of AttMtuExchangeObserver
        void ExchangedMaxAttMtuSize() override;

    private:
        void PerformDescriptorOperation();

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

        struct ReadOperation
        {
            const infra::Function<void(const infra::ConstByteRange&)> onRead;
            const infra::Function<void(uint8_t)> onDone;
        };

        struct WriteOperation
        {
            infra::ConstByteRange data;
            const infra::Function<void(uint8_t)> onDone;
        };

        struct WriteWithoutResponseOperation
        {
            infra::ConstByteRange data;
        };

        struct DescriptorOperation
        {
            const infra::Function<void(uint8_t)> onDone;
            infra::Function<void(const infra::Function<void(uint8_t)>&)> operation;
        };

        struct CharacteristicOperation
        {
            using Operation = std::variant<ReadOperation, WriteOperation, WriteWithoutResponseOperation, DescriptorOperation>;

            CharacteristicOperation(Operation operation, AttAttribute::Handle handle)
                : operation(operation)
                , handle(handle){};

            Operation operation;
            AttAttribute::Handle handle;
        };

        std::optional<std::variant<DiscoveredService, DiscoveredCharacteristic, DiscoveredDescriptor, HandleRange>> discoveryContext;
        std::optional<CharacteristicOperation> characteristicOperationContext;

        infra::ClaimableResource resource;
        infra::ClaimableResource::Claimer characteristicOperationsClaimer{ resource };
        infra::ClaimableResource::Claimer discoveryClaimer{ resource };
        infra::ClaimableResource::Claimer attMtuExchangeClaimer{ resource };
    };
}

#endif
