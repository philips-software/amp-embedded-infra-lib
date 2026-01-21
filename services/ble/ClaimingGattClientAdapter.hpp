#ifndef SERVICES_GATT_CLIENT_DECORATOR_HPP
#define SERVICES_GATT_CLIENT_DECORATOR_HPP

#include "infra/event/ClaimableResource.hpp"
#include "infra/util/AutoResetFunction.hpp"
#include "infra/util/Function.hpp"
#include "optional"
#include "services/ble/Gap.hpp"
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
        , public GattClientMtuExchange
        , private GattClientObserver
        , private AttMtuExchangeObserver
        , private GapCentralObserver
    {
    public:
        ClaimingGattClientAdapter(GattClient& gattClient, AttMtuExchange& attMtuExchange, GapCentral& gapCentral);

        // Implementation of GattClientDiscovery
        void StartServiceDiscovery() override;
        void StartCharacteristicDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
        void StartDescriptorDiscovery(AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;

        // Implementation of GattClientCharacteristicOperations
        void Read(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone) override;
        void Write(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) override;
        void WriteWithoutResponse(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) override;
        void EnableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) override;
        void DisableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) override;
        void EnableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) override;
        void DisableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) override;

        // Implementation of GattClientMtuExchange
        virtual void MtuExchange(const infra::Function<void(OperationStatus)>& onDone) override;

        // Implementation of AttMtuExchange
        uint16_t EffectiveAttMtuSize() const override;

    private:
        // Implementation of GattClientObserver
        void ServiceDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle endHandle) override;
        void ServiceDiscoveryComplete(OperationStatus status) override;
        void CharacteristicDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle, AttAttribute::Handle valueHandle, GattCharacteristic::PropertyFlags properties) override;
        void CharacteristicDiscoveryComplete(OperationStatus status) override;
        void DescriptorDiscovered(const AttAttribute::Uuid& type, AttAttribute::Handle handle) override;
        void DescriptorDiscoveryComplete(OperationStatus status) override;
        void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) override;
        void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) override;

        // Implementation of AttMtuExchangeObserver
        void ExchangedAttMtuSize() override;

        // Implementation of GapCentralObserver
        void DeviceDiscovered(const GapAdvertisingReport& deviceDiscovered) override;
        void StateChanged(GapState state) override;

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
            const infra::Function<void(OperationStatus)> onDone;
        };

        struct WriteOperation
        {
            infra::ConstByteRange data;
            const infra::Function<void(OperationStatus)> onDone;
        };

        struct DescriptorOperation
        {
            const infra::Function<void(OperationStatus)> onDone;
            infra::Function<void(const infra::Function<void(OperationStatus)>&)> operation;
        };

        struct CharacteristicOperation
        {
            using Operation = std::variant<ReadOperation, WriteOperation, DescriptorOperation>;

            CharacteristicOperation(Operation operation, AttAttribute::Handle handle)
                : operation(operation)
                , handle(handle)
            {}

            Operation operation;
            AttAttribute::Handle handle;
        };

        std::optional<std::variant<DiscoveredService, DiscoveredCharacteristic, DiscoveredDescriptor, HandleRange>> discoveryContext;
        std::optional<CharacteristicOperation> characteristicOperationContext;
        infra::AutoResetFunction<void(OperationStatus)> mtuExchangeOnDone;

        infra::ClaimableResource resource;
        infra::ClaimableResource::Claimer characteristicOperationsClaimer{ resource };
        infra::ClaimableResource::Claimer discoveryClaimer{ resource };
        infra::ClaimableResource::Claimer attMtuExchangeClaimer{ resource };
    };
}

#endif
