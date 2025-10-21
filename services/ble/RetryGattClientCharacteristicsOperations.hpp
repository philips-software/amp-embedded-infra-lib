#ifndef SERVICES_RETRY_GATT_CHARACTERISTIC_IMPL_HPP
#define SERVICES_RETRY_GATT_CHARACTERISTIC_IMPL_HPP

#include "services/ble/GattClient.hpp"
#include <optional>

namespace services
{
    class RetryGattClientCharacteristicsOperations
        : public GattClientCharacteristicOperations
        , public GattClientStackUpdateObserver
    {
    public:
        explicit RetryGattClientCharacteristicsOperations(GattClientCharacteristicOperations& gattClient);

        // Implementation of GattClientCharacteristicOperations
        void Read(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone) override;
        void Write(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) override;
        void WriteWithoutResponse(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) override;

        void EnableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) override;
        void DisableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) override;

        void EnableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) override;
        void DisableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone) override;

        // Implementation of GattClientStackUpdateObserver
        void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) override;
        void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) override;

    private:
        void TryWriteWithoutResponse();

    private:
        struct Operation
        {
            AttAttribute::Handle handle;
            infra::ConstByteRange data;
            const infra::Function<void(OperationStatus)> onDone;
        };

        GattClientCharacteristicOperations& gattClient;
        std::optional<Operation> operationWriteWithoutResponse;
    };
}
#endif
