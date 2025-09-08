#ifndef SERVICES_RETRY_GATT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_RETRY_GATT_CHARACTERISTIC_IMPL_HPP

#include "services/ble/ClaimingGattClientAdapter.hpp"
#include "services/ble/GattClient.hpp"

namespace services
{
    class RetryGattClientCharacteristicsOperations
        : public GattClientCharacteristicOperations
        , public GattClientStackUpdateObserver
    {
    public:
        explicit RetryGattClientCharacteristicsOperations(GattClientCharacteristicOperations& gattClient);

        // Implementation of GattClientCharacteristicOperations
        void Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone) override;
        void Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone) override;
        void WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) override;

        void EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;

        void EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;
        void DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone) override;

        // Implementation of GattClientStackUpdateObserver
        void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) override;
        void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) override;

    private:
        void TryWriteWithoutResponse();

    private:
        struct Operation
        {
            const GattClientCharacteristicOperationsObserver& characteristic;
            infra::ConstByteRange data;
            const infra::Function<void(OperationStatus)> onDone;
        };

        GattClientCharacteristicOperations& gattClient;
        std::optional<Operation> operationWriteWithoutResponse;
    };
}
#endif
