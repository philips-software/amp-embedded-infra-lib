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
        void ReadCharacteristic(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone) override;
        void WriteCharacteristic(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) override;
        void WriteCharacteristicWithoutResponse(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) override;
        void ReadDescriptor(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone) override;
        void WriteDescriptor(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone) override;

        // Implementation of GattClientStackUpdateObserver
        void NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data) override;
        void IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone) override;

    private:
        void TryWriteCharacteristicWithoutResponse();

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
