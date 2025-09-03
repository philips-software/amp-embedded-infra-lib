#include "services/ble/RetryGattClientCharacteristicsOperations.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    RetryGattClientCharacteristicsOperations::RetryGattClientCharacteristicsOperations(GattClientCharacteristicOperations& gattClient)
        : GattClientStackUpdateObserver(gattClient)
        , gattClient(gattClient)
    {}

    void RetryGattClientCharacteristicsOperations::Read(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(uint8_t)>& onDone)
    {
        gattClient.Read(characteristic, onRead, onDone);
    }

    void RetryGattClientCharacteristicsOperations::Write(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(uint8_t)>& onDone)
    {
        gattClient.Write(characteristic, data, onDone);
    }

    void RetryGattClientCharacteristicsOperations::WriteWithoutResponse(const GattClientCharacteristicOperationsObserver& characteristic, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        operationWriteWithoutResponse.emplace(Operation{ characteristic, data, onDone });
        TryWriteWithoutResponse();
    }

    void RetryGattClientCharacteristicsOperations::EnableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        gattClient.EnableNotification(characteristic, onDone);
    }

    void RetryGattClientCharacteristicsOperations::DisableNotification(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        gattClient.DisableNotification(characteristic, onDone);
    }

    void RetryGattClientCharacteristicsOperations::EnableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        gattClient.EnableIndication(characteristic, onDone);
    }

    void RetryGattClientCharacteristicsOperations::DisableIndication(const GattClientCharacteristicOperationsObserver& characteristic, const infra::Function<void(uint8_t)>& onDone)
    {
        gattClient.DisableIndication(characteristic, onDone);
    }

    void RetryGattClientCharacteristicsOperations::NotificationReceived(AttAttribute::Handle handle, infra::ConstByteRange data)
    {
        infra::Subject<GattClientStackUpdateObserver>::NotifyObservers([handle, data](auto& obs)
            {
                obs.NotificationReceived(handle, data);
            });
    }

    void RetryGattClientCharacteristicsOperations::IndicationReceived(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void()>& onDone)
    {
        std::tuple<AttAttribute::Handle, infra::ConstByteRange, infra::Function<void()>> context{ handle, data, onDone };
        infra::Subject<GattClientStackUpdateObserver>::NotifyObservers([&context](auto& observer)
            {
                observer.IndicationReceived(std::get<0>(context), std::get<1>(context), std::get<2>(context));
            });
    }

    void RetryGattClientCharacteristicsOperations::TryWriteWithoutResponse()
    {
        gattClient.WriteWithoutResponse(operationWriteWithoutResponse->characteristic, operationWriteWithoutResponse->data, [this](OperationStatus result)
            {
                if (result == OperationStatus::retry)
                {
                    infra::EventDispatcher::Instance().Schedule([this]()
                        {
                            TryWriteWithoutResponse();
                        });
                }
                else
                {
                    operationWriteWithoutResponse->onDone(result);
                    operationWriteWithoutResponse = std::nullopt;
                }
            });
    }
}
