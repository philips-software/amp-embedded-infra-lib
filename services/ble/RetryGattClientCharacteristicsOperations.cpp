#include "services/ble/RetryGattClientCharacteristicsOperations.hpp"
#include "infra/event/EventDispatcher.hpp"

namespace services
{
    RetryGattClientCharacteristicsOperations::RetryGattClientCharacteristicsOperations(GattClientCharacteristicOperations& gattClient)
        : GattClientStackUpdateObserver(gattClient)
        , gattClient(gattClient)
    {}

    void RetryGattClientCharacteristicsOperations::ReadCharacteristic(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.ReadCharacteristic(handle, onRead, onDone);
    }

    void RetryGattClientCharacteristicsOperations::WriteCharacteristic(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.WriteCharacteristic(handle, data, onDone);
    }

    void RetryGattClientCharacteristicsOperations::WriteCharacteristicWithoutResponse(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        operationWriteWithoutResponse.emplace(Operation{ handle, data, onDone });
        TryWriteCharacteristicWithoutResponse();
    }

    void RetryGattClientCharacteristicsOperations::ReadDescriptor(AttAttribute::Handle handle, const infra::Function<void(const infra::ConstByteRange&)>& onRead, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.ReadDescriptor(handle, onRead, onDone);
    }

    void RetryGattClientCharacteristicsOperations::WriteDescriptor(AttAttribute::Handle handle, infra::ConstByteRange data, const infra::Function<void(OperationStatus)>& onDone)
    {
        gattClient.WriteDescriptor(handle, data, onDone);
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

    void RetryGattClientCharacteristicsOperations::TryWriteCharacteristicWithoutResponse()
    {
        gattClient.WriteCharacteristicWithoutResponse(operationWriteWithoutResponse->handle, operationWriteWithoutResponse->data, [this](OperationStatus result)
            {
                if (result == OperationStatus::retry)
                {
                    infra::EventDispatcher::Instance().Schedule([this]()
                        {
                            TryWriteCharacteristicWithoutResponse();
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
