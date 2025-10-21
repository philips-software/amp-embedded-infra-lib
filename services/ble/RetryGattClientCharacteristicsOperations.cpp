#include "services/ble/RetryGattClientCharacteristicsOperations.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    RetryGattClientCharacteristicsOperations::RetryGattClientCharacteristicsOperations(GattClientCharacteristicOperations& gattClient)
        : GattClientStackUpdateObserver(gattClient)
        , gattClient(gattClient)
    {}

    void RetryGattClientCharacteristicsOperations::Read(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.Read(handle, onRead, onDone);
    }

    void RetryGattClientCharacteristicsOperations::Write(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.Write(handle, data, onDone);
    }

    void RetryGattClientCharacteristicsOperations::WriteWithoutResponse(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        operationWriteWithoutResponse.emplace(Operation{ handle, data, onDone });
        TryWriteWithoutResponse();
    }

    void RetryGattClientCharacteristicsOperations::EnableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.EnableNotification(handle, onDone);
    }

    void RetryGattClientCharacteristicsOperations::DisableNotification(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.DisableNotification(handle, onDone);
    }

    void RetryGattClientCharacteristicsOperations::EnableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.EnableIndication(handle, onDone);
    }

    void RetryGattClientCharacteristicsOperations::DisableIndication(AttAttribute::Handle handle, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.DisableIndication(handle, onDone);
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
        gattClient.WriteWithoutResponse(operationWriteWithoutResponse->handle, operationWriteWithoutResponse->data, [this](OperationStatus result)
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
